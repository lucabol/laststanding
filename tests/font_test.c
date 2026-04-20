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

    TEST_FUNCTION("8x12 default font (1.5x line-height tier)");
    {
        // Metrics: 12 rows tall, 8 wide, advance 8.
        TEST_ASSERT(l_font8x12_default.cell_h == 12, "cell_h == 12");
        TEST_ASSERT(l_font8x12_default.cell_w == 8,  "cell_w == 8");
        TEST_ASSERT(l_font8x12_default.advance == 8, "advance == 8");

        // 'A' present, not a fallback to '?'.
        L_FontGlyph g = l_font_lookup(&l_font8x12_default, 'A');
        TEST_ASSERT(g.bitmap != 0,                          "A present");
        TEST_ASSERT(g.bitmap != &l_font8x12['?' - 32][0],   "A not fallback");
        TEST_ASSERT(g.bitmap == &l_font8x12['A' - 32][0],   "A points at 8x12 bitmap");
        TEST_ASSERT(g.advance == 8,                         "A advance == 8");

        // Row-doubling invariant: generator pattern is {0,0,1,2,2,3,4,4,5,6,6,7}
        // over the 8x8 source. Rows 0..11 of 'A' must match that remap.
        const uint8_t *s = &l_font8x8['A' - 32][0];
        const uint8_t *d = g.bitmap;
        const int remap[12] = {0,0,1,2,2,3,4,4,5,6,6,7};
        int ok = 1;
        for (int i = 0; i < 12; i++) if (d[i] != s[remap[i]]) { ok = 0; break; }
        TEST_ASSERT(ok, "8x12 'A' is row-doubled 8x8 'A'");

        // Missing codepoint falls back to '?' (12-row bitmap).
        g = l_font_lookup(&l_font8x12_default, 0x00E9);
        TEST_ASSERT(g.bitmap == &l_font8x12['?' - 32][0],   "fallback to '?' in 12-row table");
    }

    TEST_FUNCTION("8x12 render: pixels in rows 8..11");
    {
        // Use a 16-wide x 16-tall canvas; draw 'H' which has vertical strokes
        // on cols 0 and 5 (from 0x33 pattern). With row-doubling, rows 10/11
        // (source row 6) carry those strokes — prove the renderer actually
        // touches rows 8..11, not just the first 8.
        uint32_t pix[16 * 16];
        for (int i = 0; i < 16 * 16; i++) pix[i] = 0;
        L_Canvas c;
        l_memset(&c, 0, sizeof(c));
        c.width = 16; c.height = 16; c.stride = 16 * 4; c.pixels = pix;

        int adv = l_draw_glyph_f(&c, &l_font8x12_default, 0, 0, 'H', 0xFFFFFFFFu);
        TEST_ASSERT(adv == 8, "H advance from 12-row drawer");

        int any_lower = 0;
        for (int row = 8; row < 12; row++)
            for (int col = 0; col < 8; col++)
                if (pix[row * 16 + col] == 0xFFFFFFFFu) { any_lower = 1; break; }
        TEST_ASSERT(any_lower, "H drew at least one pixel in rows 8..11");

        // Line height is taller than 8: two lines of 8x12 must not overlap
        // if placed 12 px apart.
        for (int i = 0; i < 16 * 16; i++) pix[i] = 0;
        l_draw_glyph_f(&c, &l_font8x12_default, 0, 0,  'H', 0xFFFFFFFFu);
        l_draw_glyph_f(&c, &l_font8x12_default, 0, 12, 'H', 0xFFFFFFFFu);
        // y=12 is outside the 16-tall canvas for rows 12..15 (still drawable 4 rows).
        // What we assert: pixels in rows 12..15 appear only from the SECOND draw.
        int any_second = 0;
        for (int row = 12; row < 16; row++)
            for (int col = 0; col < 8; col++)
                if (pix[row * 16 + col] == 0xFFFFFFFFu) { any_second = 1; break; }
        TEST_ASSERT(any_second, "second 12-row glyph drew below first");
    }

#ifdef L_FONT_PROPORTIONAL
    TEST_FUNCTION("8x12 proportional");
    {
        L_FontGlyph gi = l_font_lookup(&l_font8x12_proportional, 'i');
        L_FontGlyph gw = l_font_lookup(&l_font8x12_proportional, 'W');
        TEST_ASSERT(gi.advance < gw.advance, "i narrower than W (12-row)");

        // Widths table is shared with 8x8 proportional.
        L_FontGlyph gi8 = l_font_lookup(&l_font8x8_proportional, 'i');
        TEST_ASSERT(gi.advance == gi8.advance, "shared widths table");
    }
#endif

#ifdef L_FONT_LATIN1_SUPPLEMENT
    TEST_FUNCTION("8x12 Latin-1");
    {
        TEST_ASSERT(l_font8x12_latin1.cell_h == 12, "latin1 cell_h == 12");

        L_FontGlyph g = l_font_lookup(&l_font8x12_latin1, 0x00E9);  // é
        TEST_ASSERT(g.bitmap != 0,                           "é present");
        TEST_ASSERT(g.bitmap != &l_font8x12['?' - 32][0],    "é not fallback");
        TEST_ASSERT(g.bitmap ==
                    &l_font8x12_latin1_supp_data[0x00E9 - 0x00A0][0],
                    "é points at 12-row latin1 data, not 8-row");

        // ASCII range must ALSO resolve to 12-row data (mixing rows would
        // break the cell_h indexing).
        g = l_font_lookup(&l_font8x12_latin1, 'A');
        TEST_ASSERT(g.bitmap == &l_font8x12['A' - 32][0],    "ASCII served from 12-row table");
    }
#endif

#ifdef L_FONT_BOX_DRAWING
    TEST_FUNCTION("8x12 Box drawing");
    {
        TEST_ASSERT(l_font8x12_box.cell_h == 12,             "box cell_h == 12");

        L_FontGlyph g = l_font_lookup(&l_font8x12_box, 0x2588);  // █
        TEST_ASSERT(g.bitmap != 0,                           "█ present");
        TEST_ASSERT(g.bitmap[0] == 0xFF && g.bitmap[11] == 0xFF,
                    "█ all 12 rows filled");

        // ASCII range must resolve to 12-row data.
        g = l_font_lookup(&l_font8x12_box, 'A');
        TEST_ASSERT(g.bitmap == &l_font8x12['A' - 32][0],    "ASCII served from 12-row table");
    }
#endif

    return 0;
}
