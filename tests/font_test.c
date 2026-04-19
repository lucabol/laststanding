// font_test — smoke test for L_Font / UTF-8 / extended Unicode ranges
#define L_MAINFILE
#define L_FONT_LATIN1_SUPPLEMENT
#define L_FONT_BOX_DRAWING
#define L_FONT_PROPORTIONAL
#include "l_gfx.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    TEST_FUNCTION("UTF-8 decoder");
    {
        const char *s = "A\xC3\xA9\xE2\x86\x92!";  // "Aé→!"
        const char *p = s;
        TEST_ASSERT(l_utf8_next(&p) == (uint32_t)'A',  "ascii A");
        TEST_ASSERT(l_utf8_next(&p) == 0x00E9u,        "U+00E9 é");
        TEST_ASSERT(l_utf8_next(&p) == 0x2192u,        "U+2192 →");
        TEST_ASSERT(l_utf8_next(&p) == (uint32_t)'!',  "ascii !");
        TEST_ASSERT(l_utf8_next(&p) == 0,              "EOS returns 0");
    }
    {
        const char *bad = "\xC0\x80";  // overlong NUL
        const char *p = bad;
        TEST_ASSERT(l_utf8_next(&p) == 0xFFFDu, "overlong rejected");
    }
    {
        const char *trunc = "\xC3";  // truncated 2-byte
        const char *p = trunc;
        TEST_ASSERT(l_utf8_next(&p) == 0xFFFDu, "truncated rejected");
    }

    TEST_FUNCTION("Default font lookup");
    {
        L_FontGlyph g = l_font_lookup(&l_font8x8_default, 'A');
        TEST_ASSERT(g.bitmap != 0,                          "A present");
        TEST_ASSERT(g.bitmap == &l_font8x8['A' - 32][0],    "A points at canonical bitmap");
        TEST_ASSERT(g.advance == 8,                         "A advance == 8 (fixed)");

        g = l_font_lookup(&l_font8x8_default, 0x00E9);
        TEST_ASSERT(g.bitmap == &l_font8x8['?' - 32][0],    "fallback to '?' for missing cp");
    }

    TEST_FUNCTION("Latin-1 supplement");
    {
        L_FontGlyph g = l_font_lookup(&l_font8x8_latin1, 0x00E9);  // é
        TEST_ASSERT(g.bitmap != 0,                          "é present");
        TEST_ASSERT(g.bitmap != &l_font8x8['?' - 32][0],    "é not fallback");
        TEST_ASSERT(g.advance == 8,                         "é advance == 8");

        g = l_font_lookup(&l_font8x8_latin1, 0x00A9);  // ©
        TEST_ASSERT(g.bitmap != 0,                          "© present");

        g = l_font_lookup(&l_font8x8_latin1, 'A');
        TEST_ASSERT(g.bitmap == &l_font8x8['A' - 32][0],    "ASCII still served");
    }

    TEST_FUNCTION("Proportional widths");
    {
        L_FontGlyph gi = l_font_lookup(&l_font8x8_proportional, 'i');
        L_FontGlyph gw = l_font_lookup(&l_font8x8_proportional, 'W');
        TEST_ASSERT(gi.advance < gw.advance,                "i narrower than W");
        TEST_ASSERT(gi.advance >= 2 && gi.advance <= 8,     "i advance sane");
        TEST_ASSERT(gw.advance >= 2 && gw.advance <= 8,     "W advance sane");

        int wfix = l_text_width_f(&l_font8x8_default,      "iiii");
        int wprop = l_text_width_f(&l_font8x8_proportional, "iiii");
        TEST_ASSERT(wfix == 32,                             "fixed: 4*8");
        TEST_ASSERT(wprop < wfix,                           "proportional narrower");
    }

    TEST_FUNCTION("Box drawing (sparse)");
    {
        L_FontGlyph g = l_font_lookup(&l_font8x8_box, 0x2500);  // ─
        TEST_ASSERT(g.bitmap != 0,                          "U+2500 present");

        g = l_font_lookup(&l_font8x8_box, 0x2588);  // █
        TEST_ASSERT(g.bitmap != 0,                          "U+2588 present");
        TEST_ASSERT(g.bitmap[0] == 0xFF,                    "█ first row all set");

        g = l_font_lookup(&l_font8x8_box, 0x2501);  // not in subset
        TEST_ASSERT(g.bitmap == &l_font8x8['?' - 32][0],    "missing cp -> fallback");

        // ASCII still works
        g = l_font_lookup(&l_font8x8_box, 'A');
        TEST_ASSERT(g.bitmap == &l_font8x8['A' - 32][0],    "ASCII A served");
    }

    TEST_FUNCTION("Render to in-memory canvas");
    {
        // Build a fake canvas backed by stack pixels.
        uint32_t pix[16 * 16];
        for (int i = 0; i < 16 * 16; i++) pix[i] = 0;
        L_Canvas c;
        l_memset(&c, 0, sizeof(c));
        c.width = 16; c.height = 16; c.stride = 16 * 4; c.pixels = pix;

        int adv = l_draw_glyph_f(&c, &l_font8x8_default, 0, 0, 'A', 0xFFFFFFFFu);
        TEST_ASSERT(adv == 8, "A advance from drawer");
        // Check at least one pixel was drawn somewhere in the 8x8 cell.
        int any = 0;
        for (int i = 0; i < 8 * 16; i++) if (pix[i] == 0xFFFFFFFFu) { any = 1; break; }
        TEST_ASSERT(any, "A drew at least one pixel");

        // Latin-1 'é' draws at least one pixel too.
        for (int i = 0; i < 16 * 16; i++) pix[i] = 0;
        adv = l_draw_glyph_f(&c, &l_font8x8_latin1, 0, 0, 0x00E9, 0xFFFFFFFFu);
        TEST_ASSERT(adv == 8, "é advance");
        any = 0;
        for (int i = 0; i < 16 * 16; i++) if (pix[i] == 0xFFFFFFFFu) { any = 1; break; }
        TEST_ASSERT(any, "é drew at least one pixel");

        // UTF-8 string "ué" via l_draw_text_f advances 16 px (2 glyphs).
        for (int i = 0; i < 16 * 16; i++) pix[i] = 0;
        int total = l_draw_text_f(&c, &l_font8x8_latin1, 0, 0,
                                  "u\xC3\xA9", 0xFFFFFFFFu);
        TEST_ASSERT(total == 16, "two-glyph UTF-8 string advance");
    }

    return 0;
}
