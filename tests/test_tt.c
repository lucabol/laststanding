// tests/test_tt.c — Smoke test for l_tt.h TrueType font rendering
//
// This test focuses on API wiring and error paths. It does not embed a full
// TTF file (fonts are 10+ KB and would bloat the binary for little benefit).
// Real font rendering is exercised by examples that load fonts from disk.

#define L_MAINFILE
#include "l_tt.h"
#include "test_support.h"

TEST_DECLARE_COUNTERS();

static void test_init_rejects_bad_data(void) {
    stbtt_fontinfo info;
    l_memset(&info, 0, sizeof(info));

    TEST_CHECK(l_tt_init_font(0,   0, &info) == 0,      "NULL data rejected");
    TEST_CHECK(l_tt_init_font(0, 100, &info) == 0,      "NULL data w/ len rejected");

    unsigned char garbage[32] = { 0 };
    TEST_CHECK(l_tt_init_font(garbage,  0, &info) == 0, "zero length rejected");
    TEST_CHECK(l_tt_init_font(garbage, -1, &info) == 0, "negative length rejected");

    /* 32 zero bytes do not form a valid font. stbtt_GetFontOffsetForIndex
       either returns -1 (we catch it) or stbtt_InitFont returns 0. */
    int ok = l_tt_init_font(garbage, (int)sizeof(garbage), &info);
    TEST_CHECK(ok == 0, "zero-filled data rejected");

    /* Bogus but TTF-looking prefix — starts with the OpenType "OTTO" tag but
       has no valid tables. InitFont should fail. */
    unsigned char fake_otf[32] = {
        'O','T','T','O',   0,0,0,1,   0,0,0,0,   0,0,0,0,
        0,0,0,0,           0,0,0,0,   0,0,0,0,   0,0,0,0
    };
    ok = l_tt_init_font(fake_otf, (int)sizeof(fake_otf), &info);
    TEST_CHECK(ok == 0, "header-only fake OTF rejected");
}

static void test_render_null_info_returns_null(void) {
    int w = 99, h = 99, xo = 99, yo = 99;
    unsigned char *bm = l_tt_render_glyph(0, 'A', 16.0f, &w, &h, &xo, &yo);
    TEST_CHECK(bm == 0, "NULL info → NULL bitmap");
    TEST_CHECK(w == 0,  "w cleared on failure");
    TEST_CHECK(h == 0,  "h cleared on failure");
}

static void test_blit_handles_null_args(void) {
    uint32_t dst[4 * 4] = { 0 };
    unsigned char alpha[2 * 2] = { 255, 128, 64, 0 };

    /* NULL dst / alpha — should no-op, not crash. */
    l_tt_blit_alpha(0, 4, 4, 16, 0, 0, alpha, 2, 2, 0xFFFFFFFFu);
    l_tt_blit_alpha(dst, 4, 4, 16, 0, 0, 0, 2, 2, 0xFFFFFFFFu);
    TEST_CHECK(dst[0] == 0, "NULL alpha left dst untouched");
}

static void test_blit_writes_pixels(void) {
    uint32_t dst[4 * 4];
    for (int i = 0; i < 16; i++) dst[i] = 0;

    /* 2x2 alpha: full, half, quarter, zero. Color = opaque white. */
    unsigned char alpha[2 * 2] = { 255, 128, 64, 0 };
    l_tt_blit_alpha(dst, 4, 4, 16, 1, 1, alpha, 2, 2, 0xFFFFFFFFu);

    /* (1,1) should be white (alpha=255). */
    TEST_CHECK(dst[1 * 4 + 1] == 0xFFFFFFFFu, "full-alpha pixel is white");
    /* (2,1) should be partially opaque grey (alpha=128). */
    uint32_t p = dst[1 * 4 + 2];
    unsigned a = (p >> 24) & 0xFF;
    TEST_CHECK(a > 0 && a < 255, "half-alpha pixel is partially opaque");
    /* (1,2) quarter-alpha pixel (alpha=64) is still nonzero. */
    p = dst[2 * 4 + 1];
    TEST_CHECK(((p >> 24) & 0xFF) > 0, "quarter-alpha pixel is nonzero");
    /* (2,2) zero-alpha leaves dst as 0. */
    TEST_CHECK(dst[2 * 4 + 2] == 0, "zero-alpha skipped");
    /* Outside the blit rect — untouched. */
    TEST_CHECK(dst[0] == 0, "corner (0,0) untouched");
    TEST_CHECK(dst[15] == 0, "corner (3,3) untouched");
}

static void test_blit_clips(void) {
    uint32_t dst[4 * 4];
    for (int i = 0; i < 16; i++) dst[i] = 0;

    unsigned char alpha[3 * 3] = {
        255, 255, 255,
        255, 255, 255,
        255, 255, 255
    };
    /* Place near bottom-right so most of the 3x3 clips off the 4x4 canvas. */
    l_tt_blit_alpha(dst, 4, 4, 16, 3, 3, alpha, 3, 3, 0xFF00FF00u);
    /* Only (3,3) should be written; (4,3), (3,4), (4,4) are out of bounds. */
    TEST_CHECK(dst[3 * 4 + 3] == 0xFF00FF00u, "in-bounds pixel drawn");
    TEST_CHECK(dst[3 * 4 + 2] == 0,           "left neighbor untouched (out of alpha)");
}

static void test_allocator_save_restore(void) {
    /* Allocate, save, allocate more, restore — the second batch's memory
       should be reclaimed. We can't observe pool internals directly, but we
       can verify allocations still succeed after restore. */
    void *a = l_tt_malloc(128);
    TEST_CHECK(a != 0, "first alloc succeeds");
    size_t mark = l_tt__save();
    void *b = l_tt_malloc(256);
    TEST_CHECK(b != 0, "second alloc succeeds");
    l_tt__restore(mark);
    void *c = l_tt_malloc(64);
    TEST_CHECK(c != 0, "alloc after restore succeeds");
    /* c should be at or before where b was, since we restored. */
    TEST_CHECK((unsigned char *)c <= (unsigned char *)b,
               "restore reclaimed pool space");
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    puts("Testing l_tt.h TrueType wrapper...\n");

    test_init_rejects_bad_data();
    test_render_null_info_returns_null();
    test_blit_handles_null_args();
    test_blit_writes_pixels();
    test_blit_clips();
    test_allocator_save_restore();

    l_test_print_summary(passed_count, test_count);
    if (passed_count != test_count) { puts("FAIL\n"); exit(1); }
    puts("PASS\n");
    return 0;
}
