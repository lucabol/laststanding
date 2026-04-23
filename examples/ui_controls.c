#define L_WITHSNPRINTF
#define L_MAINFILE
#include "l_ui.h"

// ui_controls — RGB color mixer demo
// Usage: ui_controls
// Exit: press Escape

#define WIN_W 480
#define WIN_H 360

static void color_to_hex(int r, int g, int b, char *out) {
    out[0] = '#';
    unsigned char rgb[3] = {(unsigned char)r, (unsigned char)g, (unsigned char)b};
    l_bin2hex(out + 1, rgb, 3);
    for (int i = 1; i <= 6; i++) out[i] = (char)l_toupper(out[i]);
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

static int hex_to_rgb(const char *hex, int *r, int *g, int *b) {
    if (!hex || hex[0] != '#') return 0;
    if (l_strlen(hex) != 7) return 0;
    int rh = hex_val(hex[1]), rl = hex_val(hex[2]);
    int gh = hex_val(hex[3]), gl = hex_val(hex[4]);
    int bh = hex_val(hex[5]), bl = hex_val(hex[6]);
    if (rh < 0 || rl < 0 || gh < 0 || gl < 0 || bh < 0 || bl < 0) return 0;
    *r = rh * 16 + rl;
    *g = gh * 16 + gl;
    *b = bh * 16 + bl;
    return 1;
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas canvas;
    if (l_canvas_open(&canvas, WIN_W, WIN_H, "Color Mixer") != 0) {
        puts("Could not open canvas\n");
        return 1;
    }
    int s = canvas.scale;  // HiDPI multiplier for hardcoded coords.

    L_UI ui;
    l_ui_init(&ui);  // font_scale auto-picked from canvas.scale in l_ui_begin.

    int red = 128, green = 64, blue = 200;
    char hex_buf[L_UI_MAX_TEXT];
    color_to_hex(red, green, blue, hex_buf);

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(25, 25, 28));

        if (ui.key == 27) break;

        int cw = canvas.width, ch = canvas.height;

        // Panel
        l_ui_panel(&ui, 10 * s, 10 * s, cw - 20 * s, ch - 20 * s);

        // Title
        {
            int saved = ui.font_scale;
            ui.font_scale = 2 * s;
            l_ui__draw_text(&canvas, 24 * s, 18 * s, "Color Mixer",
                            ui.theme.accent, 2 * s);
            ui.font_scale = saved;
        }

        l_ui_column_begin(&ui, 24 * s, 50 * s, 6 * s);

        int slider_w = (cw < 480 * s ? cw - 80 * s : 200 * s);
        if (cw > 300 * s) slider_w = (cw / 2) - 60 * s;
        if (slider_w < 60 * s) slider_w = 60 * s;

        // Red slider
        {
            int y = l_ui_next(&ui, 8 * s);
            char lbl[32];
            l_snprintf(lbl, sizeof(lbl), "R: %d", red);
            l_ui_label(&ui, 24 * s, y, lbl);
            y = l_ui_next(&ui, 16 * s);
            l_ui_slider(&ui, 24 * s, y, slider_w, &red, 0, 255);
        }

        // Green slider
        {
            int y = l_ui_next(&ui, 8 * s);
            char lbl[32];
            l_snprintf(lbl, sizeof(lbl), "G: %d", green);
            l_ui_label(&ui, 24 * s, y, lbl);
            y = l_ui_next(&ui, 16 * s);
            l_ui_slider(&ui, 24 * s, y, slider_w, &green, 0, 255);
        }

        // Blue slider
        {
            int y = l_ui_next(&ui, 8 * s);
            char lbl[32];
            l_snprintf(lbl, sizeof(lbl), "B: %d", blue);
            l_ui_label(&ui, 24 * s, y, lbl);
            y = l_ui_next(&ui, 16 * s);
            l_ui_slider(&ui, 24 * s, y, slider_w, &blue, 0, 255);
        }

        // Hex textbox
        {
            int y = l_ui_next(&ui, 8 * s);
            l_ui_label(&ui, 24 * s, y, "Hex:");
            y = l_ui_next(&ui, 20 * s);
            if (l_ui_textbox(&ui, 24 * s, y, 100 * s, hex_buf, L_UI_MAX_TEXT)) {
                // Parse hex -> update sliders
                int r2, g2, b2;
                if (hex_to_rgb(hex_buf, &r2, &g2, &b2)) {
                    red = r2; green = g2; blue = b2;
                }
            } else {
                // Sliders changed -> update hex
                color_to_hex(red, green, blue, hex_buf);
            }
        }

        l_ui_separator(&ui, 24 * s, l_ui_next(&ui, 4 * s) + 2 * s, cw - 68 * s);

        // Reset button
        {
            int y = l_ui_next(&ui, 26 * s);
            if (l_ui_button(&ui, 24 * s, y, 80 * s, 24 * s, "Reset")) {
                red = 128; green = 64; blue = 200;
                color_to_hex(red, green, blue, hex_buf);
            }
        }

        l_ui_layout_end(&ui);

        // Color preview rectangle
        {
            uint32_t col = L_RGB(red, green, blue);
            int px = slider_w + 60 * s, py = 50 * s;
            int pw = cw - px - 20 * s, ph = ch - 80 * s;
            if (pw > 20 * s && ph > 20 * s) {
                l_fill_rect(&canvas, px, py, pw, ph, col);
                l_rect(&canvas, px, py, pw, ph, ui.theme.border);

                // Show color value as text in contrasting color
                int lum = (red * 299 + green * 587 + blue * 114) / 1000;
                uint32_t tc = (lum > 128) ? L_BLACK : L_WHITE;
                char label[32];
                color_to_hex(red, green, blue, label);
                int tw = l_ui__text_width(label, s);
                l_ui__draw_text(&canvas, px + (pw - tw) / 2, py + ph / 2 - 4 * s,
                                label, tc, s);
            }
        }

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }

    l_canvas_close(&canvas);
    return 0;
}
