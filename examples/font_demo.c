// font_demo — Showcase of the L_Font / _f drawing API.
//
// Opt-in to the extended fonts with compile-time macros. The default ASCII
// font is always available; the other tables are only pulled in when their
// L_FONT_* macro is defined before including l_gfx.h. This keeps the default
// binary size unchanged for programs that don't use them.
//
// Exit: press Escape or close the window.

#define L_MAINFILE
#define L_FONT_PROPORTIONAL        // enables l_font8x8_proportional / l_font8x12_proportional
#define L_FONT_LATIN1_SUPPLEMENT   // enables l_font8x8_latin1 / l_font8x12_latin1
#define L_FONT_BOX_DRAWING         // enables l_font8x8_box    / l_font8x12_box
#include "l_gfx.h"

static void draw_section(L_Canvas *c, int x, int y, const char *title) {
    l_draw_text(c, x, y, title, L_YELLOW);
    l_line(c, x, y + 10, x + 380, y + 10, 0xFF404040);
}

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    L_Canvas c;
    if (l_canvas_open(&c, 820, 600, "Font Demo — l_gfx.h L_Font API") != 0)
        return 1;

    while (l_canvas_alive(&c)) {
        l_canvas_clear(&c, 0xFF101018);

        // ── Left column: fixed-width vs proportional ────────────────────
        int x = 20, y = 16;

        draw_section(&c, x, y, "Default 8x8 font (fixed width):");
        l_draw_text_f(&c, &l_font8x8_default, x, y + 18,
                      "The quick brown fox jumps over the lazy dog.",
                      L_WHITE);
        l_draw_text_f(&c, &l_font8x8_default, x, y + 30,
                      "iii iii iii  MMM MMM MMM   012345 6789",
                      0xFFA0A0A0);

        y += 60;
        draw_section(&c, x, y, "Proportional 8x8 font (L_FONT_PROPORTIONAL):");
        l_draw_text_f(&c, &l_font8x8_proportional, x, y + 18,
                      "The quick brown fox jumps over the lazy dog.",
                      L_WHITE);
        l_draw_text_f(&c, &l_font8x8_proportional, x, y + 30,
                      "iii iii iii  MMM MMM MMM   012345 6789",
                      0xFFA0A0A0);
        int pw = l_text_width_f(&l_font8x8_proportional,
                                "The quick brown fox jumps over the lazy dog.");
        l_draw_text(&c, x, y + 44, "proportional width:", 0xFF808080);
        char buf[32]; l_itoa(pw, buf, 10); l_strcat(buf, " px");
        l_draw_text(&c, x + 160, y + 44, buf, L_CYAN);

        // ── Latin-1 Supplement ───────────────────────────────────────────
        y += 70;
        draw_section(&c, x, y, "Latin-1 Supplement (L_FONT_LATIN1_SUPPLEMENT):");
        l_draw_text_f(&c, &l_font8x8_latin1, x, y + 18,
                      "Caf\xc3\xa9""  na\xc3\xafve  r\xc3\xa9sum\xc3\xa9  "
                      "\xc3\xb1""oche  \xc3\xbc""ber  \xc3\x87\xc3\xa7",
                      L_WHITE);
        l_draw_text_f(&c, &l_font8x8_latin1, x, y + 30,
                      "\xc2\xa1Hola!  \xc2\xbf""C\xc3\xb3mo est\xc3\xa1s?  "
                      "\xc2\xa9"" 2024  \xc2\xb1 5\xc2\xb0""C",
                      0xFFA0A0A0);

        // ── Box drawing ─────────────────────────────────────────────────
        y += 50;
        draw_section(&c, x, y, "Box drawing (L_FONT_BOX_DRAWING):");
        // A small decorative frame built from Unicode box-drawing characters.
        l_draw_text_scaled_f(&c, &l_font8x8_box, x,       y + 20,
            "\xe2\x94\x8c\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80"
            "\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80"
            "\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80"
            "\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x90",
            L_GREEN, 2, 2);
        l_draw_text_scaled_f(&c, &l_font8x8_box, x,       y + 36,
            "\xe2\x94\x82", L_GREEN, 2, 2);
        l_draw_text_scaled_f(&c, &l_font8x8_default, x + 20, y + 40,
            "laststanding", L_WHITE, 2, 2);
        l_draw_text_scaled_f(&c, &l_font8x8_box, x + 312, y + 36,
            "\xe2\x94\x82", L_GREEN, 2, 2);
        l_draw_text_scaled_f(&c, &l_font8x8_box, x,       y + 52,
            "\xe2\x94\x94\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80"
            "\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80"
            "\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80"
            "\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x80\xe2\x94\x98",
            L_GREEN, 2, 2);

        // Arrows
        l_draw_text_scaled_f(&c, &l_font8x8_box, x, y + 76,
            "\xe2\x86\x90  \xe2\x86\x92  \xe2\x86\x91  \xe2\x86\x93",
            L_CYAN, 2, 2);

        // ── Right column: scaling + fallback ─────────────────────────────
        int rx = 430, ry = 16;

        draw_section(&c, rx, ry, "Scaling (same font, sx=1..4):");
        l_draw_text_scaled_f(&c, &l_font8x8_proportional, rx, ry + 18,
                             "Proportional", L_WHITE, 1, 1);
        l_draw_text_scaled_f(&c, &l_font8x8_proportional, rx, ry + 32,
                             "Proportional", L_WHITE, 2, 2);
        l_draw_text_scaled_f(&c, &l_font8x8_proportional, rx, ry + 58,
                             "Proportional", L_WHITE, 3, 3);

        // Fallback behavior: characters outside a font's ranges render as
        // the font's fallback glyph (here '?').
        ry = 170;
        draw_section(&c, rx, ry, "Fallback (Greek \xce\xb1\xce\xb2\xce\xb3 not in font):");
        l_draw_text_f(&c, &l_font8x8_default, rx, ry + 18,
                      "ASCII + \xce\xb1\xce\xb2\xce\xb3 + \xe2\x9c\x93 => ASCII + ??? + ?",
                      L_WHITE);
        l_draw_text(&c, rx, ry + 32,
                    "(each missing codepoint is replaced once by '?')",
                    0xFF808080);

        // UTF-8 decoder showcase: byte lengths vary 1..4 bytes per codepoint.
        ry = 220;
        draw_section(&c, rx, ry, "UTF-8 decoder (l_utf8_next):");
        l_draw_text(&c, rx, ry + 18, "1-byte:  'A' = 0x41", L_WHITE);
        l_draw_text(&c, rx, ry + 30, "2-byte:  'e\xcc\x81' NFC U+00E9 = 0xC3 0xA9", L_WHITE);
        l_draw_text(&c, rx, ry + 42, "3-byte:  box U+2500 = 0xE2 0x94 0x80", L_WHITE);
        l_draw_text(&c, rx, ry + 54, "invalid -> U+FFFD (replacement)", L_WHITE);

        // Size impact reminder
        ry = 310;
        draw_section(&c, rx, ry, "Size budget:");
        l_draw_text(&c, rx, ry + 18, "hello.exe (no l_gfx.h):         unchanged", 0xFFA0FFA0);
        l_draw_text(&c, rx, ry + 30, "ui_demo (no font macros):       unchanged", 0xFFA0FFA0);
        l_draw_text(&c, rx, ry + 42, "this demo (all 3 macros):       ~+3 KB",    0xFFFFC040);

        // ── 8x12 bitmap family (1.5x line-height tier) ────────────────
        {
            int sx = 20, sy = 500;
            draw_section(&c, sx, sy, "8x12 family (1.5x line-height tier):");
            // Side-by-side comparison with matching baselines.
            l_draw_text_f(&c, &l_font8x8_default,        sx,       sy + 22,
                          "8x8  The quick brown fox 0123", L_WHITE);
            l_draw_text_f(&c, &l_font8x12_default,       sx + 400, sy + 22,
                          "8x12 The quick brown fox 0123", L_WHITE);
            l_draw_text_f(&c, &l_font8x8_latin1,         sx,       sy + 40,
                          "8x8  Caf\xc3\xa9 r\xc3\xa9sum\xc3\xa9 \xc2\xa9 na\xc3\xafve", 0xFFA0A0A0);
            l_draw_text_f(&c, &l_font8x12_latin1,        sx + 400, sy + 40,
                          "8x12 Caf\xc3\xa9 r\xc3\xa9sum\xc3\xa9 \xc2\xa9 na\xc3\xafve", 0xFFA0A0A0);
            l_draw_text_f(&c, &l_font8x8_proportional,   sx,       sy + 60,
                          "8x8  iii MMM WWW proportional", L_CYAN);
            l_draw_text_f(&c, &l_font8x12_proportional,  sx + 400, sy + 60,
                          "8x12 iii MMM WWW proportional", L_CYAN);
        }

        // Footer
        l_draw_text(&c, 20, 580, "Esc to exit  -  l_draw_text_f / l_draw_text_scaled_f / l_text_width_f",
                    0xFF606060);

        l_canvas_flush(&c);
        if (l_canvas_key(&c) == 27) break;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
