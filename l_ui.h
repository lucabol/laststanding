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

#ifdef L_UI_WITH_CUSTOM_FONT
    // Optional custom font (only compiled in when L_UI_WITH_CUSTOM_FONT is
    // defined before including l_ui.h). NULL means "use the built-in ASCII
    // path". Set to e.g. &l_font8x8_proportional or &l_font8x8_latin1 after
    // also defining L_FONT_PROPORTIONAL / L_FONT_LATIN1_SUPPLEMENT.
    const L_Font *font;
#endif
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

// Internal macros used by widgets. When L_UI_WITH_CUSTOM_FONT is defined and
// ui->font is non-NULL, widgets route through the L_Font/_f drawing API
// (UTF-8, multiple fonts, proportional widths). Otherwise they use the legacy
// ASCII-only path with zero size impact.
#ifdef L_UI_WITH_CUSTOM_FONT
#  define L_UI__DRAW_TEXT(ui_, x_, y_, txt_, col_)                               \
    do { const L_UI *_u = (ui_); const char *_t = (txt_);                        \
         if (_u->font) {                                                         \
             if (_u->font_scale <= 1) l_draw_text_f(_u->canvas, _u->font,        \
                 (x_), (y_), _t, (col_));                                        \
             else                     l_draw_text_scaled_f(_u->canvas, _u->font, \
                 (x_), (y_), _t, (col_), _u->font_scale, _u->font_scale);        \
         } else {                                                                \
             l_ui__draw_text(_u->canvas, (x_), (y_), _t, (col_), _u->font_scale);\
         }                                                                       \
    } while (0)
#  define L_UI__TEXT_WIDTH(ui_, txt_)                                            \
    ((ui_)->font                                                                 \
        ? l_text_width_f((ui_)->font, (txt_)) * (ui_)->font_scale                \
        : l_ui__text_width((txt_), (ui_)->font_scale))
#  define L_UI__TEXT_HEIGHT(ui_)                                                 \
    (((ui_)->font ? (ui_)->font->cell_h : 8) * (ui_)->font_scale)
#else
#  define L_UI__DRAW_TEXT(ui_, x_, y_, txt_, col_)                               \
    l_ui__draw_text((ui_)->canvas, (x_), (y_), (txt_), (col_), (ui_)->font_scale)
#  define L_UI__TEXT_WIDTH(ui_, txt_)                                            \
    l_ui__text_width((txt_), (ui_)->font_scale)
#  define L_UI__TEXT_HEIGHT(ui_)                                                 \
    l_ui__text_height((ui_)->font_scale)
#endif

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

// ---------------------------------------------------------------------------
// Widgets
// ---------------------------------------------------------------------------

/// Draws a text label at (x,y). Returns 0 always.
static inline int l_ui_label(L_UI *ui, int x, int y, const char *text) {
    L_UI__DRAW_TEXT(ui, x, y, text, ui->theme.fg);
    return 0;
}

/// Draws a clickable button at (x,y) with given width and height. Returns 1 if clicked this frame.
static inline int l_ui_button(L_UI *ui, int x, int y, int w, int h, const char *label) {
    uint32_t id = l_ui__hash(label, x, y);
    int over = l_ui__in_rect(ui->mouse_x, ui->mouse_y, x, y, w, h);
    int clicked = 0;

    if (over) {
        ui->hot = id;
        if (l_ui__mouse_pressed(ui))
            ui->active = id;
    }
    if (ui->active == id && l_ui__mouse_released(ui)) {
        if (over) clicked = 1;
        ui->active = 0;
    }

    // Draw
    uint32_t bg = ui->theme.bg;
    if (ui->active == id && over)      bg = ui->theme.bg_active;
    else if (ui->hot == id)            bg = ui->theme.bg_hover;

    l_fill_rect(ui->canvas, x, y, w, h, bg);
    l_rect(ui->canvas, x, y, w, h, ui->theme.border);

    // Center text
    int tw = L_UI__TEXT_WIDTH(ui, label);
    int th = L_UI__TEXT_HEIGHT(ui);
    int tx = x + (w - tw) / 2;
    int ty = y + (h - th) / 2;
    L_UI__DRAW_TEXT(ui, tx, ty, label, ui->theme.fg);

    return clicked;
}

/// Draws a checkbox at (x,y). *checked is toggled on click. Returns 1 if toggled this frame.
static inline int l_ui_checkbox(L_UI *ui, int x, int y, const char *label, int *checked) {
    int scale = ui->font_scale;
    int box = 8 * scale * 2;  // checkbox size: 16 at scale 1
    uint32_t id = l_ui__hash(label, x, y);
    int over = l_ui__in_rect(ui->mouse_x, ui->mouse_y, x, y, box, box);
    int toggled = 0;

    if (over) {
        ui->hot = id;
        if (l_ui__mouse_pressed(ui))
            ui->active = id;
    }
    if (ui->active == id && l_ui__mouse_released(ui)) {
        if (over) {
            *checked = !(*checked);
            toggled = 1;
        }
        ui->active = 0;
    }

    // Draw box
    uint32_t bg = (ui->hot == id) ? ui->theme.bg_hover : ui->theme.bg;
    l_fill_rect(ui->canvas, x, y, box, box, bg);
    l_rect(ui->canvas, x, y, box, box, ui->theme.border);

    // Draw check mark (filled inner rect)
    if (*checked) {
        int pad = box / 4;
        l_fill_rect(ui->canvas, x + pad, y + pad, box - pad * 2, box - pad * 2,
                     ui->theme.accent);
    }

    // Label to the right
    int th = L_UI__TEXT_HEIGHT(ui);
    L_UI__DRAW_TEXT(ui, x + box + 4 * scale, y + (box - th) / 2,
                    label, ui->theme.fg);

    return toggled;
}

/// Draws a horizontal slider at (x,y) with given width. *value is clamped to [min_val, max_val]. Returns 1 if value changed.
static inline int l_ui_slider(L_UI *ui, int x, int y, int w, int *value,
                               int min_val, int max_val) {
    int scale = ui->font_scale;
    int track_h = 4 * scale;
    int thumb_w = 8 * scale;
    int thumb_h = 16 * scale;
    int h = thumb_h;  // total widget height
    uint32_t id = l_ui__hash("slider", x, y);
    int over = l_ui__in_rect(ui->mouse_x, ui->mouse_y, x, y, w, h);
    int changed = 0;

    if (over) {
        ui->hot = id;
        if (l_ui__mouse_pressed(ui))
            ui->active = id;
    }

    // Clamp value
    if (*value < min_val) *value = min_val;
    if (*value > max_val) *value = max_val;

    // If active, update value from mouse position
    if (ui->active == id && (ui->mouse_btn & 1)) {
        int range = max_val - min_val;
        int usable = w - thumb_w;
        int mx = ui->mouse_x - x - thumb_w / 2;
        if (mx < 0) mx = 0;
        if (mx > usable) mx = usable;
        int new_val = min_val + (usable > 0 ? mx * range / usable : 0);
        if (new_val != *value) {
            *value = new_val;
            changed = 1;
        }
    }

    // Draw track
    int track_y = y + (h - track_h) / 2;
    l_fill_rect(ui->canvas, x, track_y, w, track_h, ui->theme.border);

    // Draw thumb
    int usable = w - thumb_w;
    int range = max_val - min_val;
    int thumb_x = x + (range > 0 ? (*value - min_val) * usable / range : 0);
    int thumb_y = y;
    uint32_t thumb_col = (ui->active == id) ? ui->theme.fg : ui->theme.accent;
    l_fill_rect(ui->canvas, thumb_x, thumb_y, thumb_w, thumb_h, thumb_col);

    return changed;
}

/// Draws a single-line text input at (x,y) with width w. buf is the text buffer, buf_len is max capacity. Returns 1 if text changed.
static inline int l_ui_textbox(L_UI *ui, int x, int y, int w, char *buf, int buf_len) {
    int scale = ui->font_scale;
    int h = l_ui__text_height(scale) + 8 * scale;
    uint32_t id = l_ui__hash("textbox", x, y);
    int over = l_ui__in_rect(ui->mouse_x, ui->mouse_y, x, y, w, h);
    int changed = 0;

    // Set hot when hovered (prevents l_ui_end from clearing focus)
    if (over)
        ui->hot = id;

    // Click to focus
    if (over && l_ui__mouse_pressed(ui)) {
        ui->focused = id;
        ui->cursor_pos = (int)l_strlen(buf);
        ui->cursor_blink = 0;
    }

    int focused = (ui->focused == id);

    // Draw background and border
    l_fill_rect(ui->canvas, x, y, w, h, ui->theme.input_bg);
    l_rect(ui->canvas, x, y, w, h, focused ? ui->theme.accent : ui->theme.border);

    // Handle keyboard input when focused
    if (focused && ui->key) {
        int len = (int)l_strlen(buf);
        int key = ui->key;
        if (key == 8) {  // backspace
            if (ui->cursor_pos > 0) {
                // Shift chars left
                for (int i = ui->cursor_pos - 1; i < len - 1; i++)
                    buf[i] = buf[i + 1];
                buf[len - 1] = '\0';
                ui->cursor_pos--;
                changed = 1;
            }
        } else if (key == 1001) {  // left arrow
            if (ui->cursor_pos > 0) ui->cursor_pos--;
        } else if (key == 1002) {  // right arrow
            if (ui->cursor_pos < len) ui->cursor_pos++;
        } else if (key >= 32 && key <= 126) {  // printable ASCII
            if (len < buf_len - 1) {
                // Shift chars right to make room
                for (int i = len; i > ui->cursor_pos; i--)
                    buf[i] = buf[i - 1];
                buf[ui->cursor_pos] = (char)key;
                buf[len + 1] = '\0';
                ui->cursor_pos++;
                changed = 1;
            }
        }
        ui->cursor_blink = 0;  // reset blink on any key
    }

    // Draw text
    int pad = 4 * scale;
    int text_y = y + (h - l_ui__text_height(scale)) / 2;
    // Clip: only draw chars that fit in the box
    int max_chars = (w - pad * 2) / (8 * scale);
    int len = (int)l_strlen(buf);
    // Scroll offset: ensure cursor is visible
    int scroll = 0;
    if (ui->cursor_pos > max_chars)
        scroll = ui->cursor_pos - max_chars;

    // Draw visible portion of text
    for (int i = scroll; i < len && (i - scroll) < max_chars; i++) {
        char ch = buf[i];
        if (ch >= 32 && ch <= 126) {
            int cx = x + pad + (i - scroll) * 8 * scale;
            l_ui__draw_text(ui->canvas, cx, text_y, (const char[2]){ch, 0},
                            ui->theme.fg, scale);
        }
    }

    // Draw blinking cursor when focused
    if (focused && (ui->cursor_blink % 60) < 30) {
        int cursor_x = x + pad + (ui->cursor_pos - scroll) * 8 * scale;
        l_fill_rect(ui->canvas, cursor_x, text_y, scale, l_ui__text_height(scale),
                     ui->theme.fg);
    }

    return changed;
}

/// Draws a panel (filled rectangle with border) at (x,y). Returns 0.
static inline int l_ui_panel(L_UI *ui, int x, int y, int w, int h) {
    l_fill_rect(ui->canvas, x, y, w, h, ui->theme.bg);
    l_rect(ui->canvas, x, y, w, h, ui->theme.border);
    return 0;
}

/// Draws a horizontal separator line at (x,y) with width w. Returns 0.
static inline int l_ui_separator(L_UI *ui, int x, int y, int w) {
    l_hline(ui->canvas, x, x + w - 1, y, ui->theme.border);
    return 0;
}

// ---------------------------------------------------------------------------
// Auto-Layout Helpers
// ---------------------------------------------------------------------------

/// Begins a vertical (column) auto-layout at (x,y) with given spacing between widgets.
static inline void l_ui_column_begin(L_UI *ui, int x, int y, int spacing) {
    ui->layout_x = x;
    ui->layout_y = y;
    ui->layout_spacing = spacing;
    ui->layout_dir = 1;
    ui->layout_active = 1;
}

/// Begins a horizontal (row) auto-layout at (x,y) with given spacing.
static inline void l_ui_row_begin(L_UI *ui, int x, int y, int spacing) {
    ui->layout_x = x;
    ui->layout_y = y;
    ui->layout_spacing = spacing;
    ui->layout_dir = 0;
    ui->layout_active = 1;
}

/// Advances auto-layout by `size` pixels. Returns the position before advancing (y for column, x for row).
static inline int l_ui_next(L_UI *ui, int size) {
    if (ui->layout_dir == 1) {
        // Vertical: return current y, advance y
        int pos = ui->layout_y;
        ui->layout_y += size + ui->layout_spacing;
        return pos;
    } else {
        // Horizontal: return current x, advance x
        int pos = ui->layout_x;
        ui->layout_x += size + ui->layout_spacing;
        return pos;
    }
}

/// Ends the current layout.
static inline void l_ui_layout_end(L_UI *ui) {
    ui->layout_active = 0;
}

#endif // L_UI_H
