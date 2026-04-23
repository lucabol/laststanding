// l_tt.h — Freestanding TrueType font rendering for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_tt.h"   // pulls in l_os.h + stb_truetype.h
//
// Wraps stb_truetype.h for freestanding use (no libc, no malloc, no stdio).
// Parses TrueType/OpenType fonts and rasterizes glyphs from memory buffers.
// Uses mmap-backed bump allocation (no heap) with a watermark save/restore
// pattern so repeated glyph rendering doesn't leak the pool.
//
// Security: stb_truetype provides NO range-checking on font offsets. Do NOT
// render untrusted fonts — an attacker-controlled font can read arbitrary
// memory.
//
// API (see also stbtt_* functions in stb_truetype.h for advanced use):
//   l_tt_init_font(data, len, info)                   → 1 on success, 0 on failure
//   l_tt_scale_for_pixel_height(info, pixels)         → float scale factor
//   l_tt_vmetrics(info, &ascent, &descent, &lineGap)  → vertical metrics
//   l_tt_hmetrics(info, cp, &advance, &lsb)           → horizontal metrics
//   l_tt_kern_advance(info, cp1, cp2)                 → kerning in unscaled units
//   l_tt_render_glyph(info, cp, scale, &w, &h, &xoff, &yoff)
//                                                     → alpha bitmap or NULL
//   l_tt_free_bitmap(bitmap, w, h)                    → release bitmap
//   l_tt_blit_alpha(dst, dw, dh, dstride, x, y,
//                   alpha, aw, ah, color)             → blit α-bitmap into ARGB
//   l_tt_draw_text(dst, dw, dh, dstride, info,
//                  x, baseline_y, pixel_height,
//                  utf8, color)                       → high-level draw, crisp
//
// The low-level path (`l_tt_render_glyph` + `l_tt_blit_alpha`) snaps every
// glyph to integer pixels and rasterizes at 1×1 — simple but soft at small
// sizes. Prefer `l_tt_draw_text` for body text: it carries the pen in float,
// rasterizes at 2× with box downsampling, and composites with a gamma ≈ 2
// stem-darkening approximation.
//
// The font's memory buffer (passed to l_tt_init_font) MUST remain valid for
// the entire lifetime of the stbtt_fontinfo — stb_truetype reads from it on
// every glyph lookup.

#ifndef L_TT_H
#define L_TT_H

#include "l_os.h"

// -- Public API forward declarations ------------------------------------------

// The stbtt_fontinfo type is defined by stb_truetype.h itself; we re-expose it.
struct stbtt_fontinfo;

static inline int   l_tt_init_font(const unsigned char *data, int len,
                                   struct stbtt_fontinfo *info);
static inline float l_tt_scale_for_pixel_height(const struct stbtt_fontinfo *info,
                                                float pixels);
static inline void  l_tt_vmetrics(const struct stbtt_fontinfo *info,
                                  int *ascent, int *descent, int *lineGap);
static inline void  l_tt_hmetrics(const struct stbtt_fontinfo *info, int codepoint,
                                  int *advance, int *lsb);
static inline int   l_tt_kern_advance(const struct stbtt_fontinfo *info,
                                      int cp1, int cp2);
static inline unsigned char *l_tt_render_glyph(const struct stbtt_fontinfo *info,
                                                int codepoint, float scale,
                                                int *w, int *h,
                                                int *xoff, int *yoff);
static inline void  l_tt_free_bitmap(unsigned char *bitmap, int w, int h);
static inline void  l_tt_blit_alpha(uint32_t *dst, int dw, int dh, int dstride,
                                    int x, int y,
                                    const unsigned char *alpha, int aw, int ah,
                                    uint32_t color);
static inline float l_tt_draw_text(uint32_t *dst, int dw, int dh, int dstride,
                                   const struct stbtt_fontinfo *info,
                                   float x, float baseline_y,
                                   float pixel_height,
                                   const char *utf8, uint32_t color);

// -- Implementation (only included once with L_WITHDEFS) ----------------------

#ifdef L_WITHDEFS

// -- Freestanding allocator for stb_truetype ----------------------------------
// stb_truetype allocates many small temporary buffers during rasterization
// (edge structures, scanlines). Since we use a bump allocator where frees are
// no-ops, we also expose save/restore watermarks so per-glyph allocations can
// be reclaimed after each bitmap is rendered.

#define L_TT_POOL_SIZE (256 * 1024 * 1024)

static unsigned char *l_tt__pool_base;
static size_t         l_tt__pool_used;
static size_t         l_tt__pool_cap;

static inline void l_tt__pool_init(void) {
    if (l_tt__pool_base) return;
    l_tt__pool_base = (unsigned char *)l_mmap(0, L_TT_POOL_SIZE,
                                              L_PROT_READ | L_PROT_WRITE,
                                              L_MAP_PRIVATE | L_MAP_ANONYMOUS,
                                              -1, 0);
    if (l_tt__pool_base == (unsigned char *)L_MAP_FAILED) l_tt__pool_base = 0;
    l_tt__pool_cap  = L_TT_POOL_SIZE;
    l_tt__pool_used = 0;
}

static inline void *l_tt_malloc(size_t sz) {
    l_tt__pool_init();
    if (!l_tt__pool_base || sz == 0) return 0;
    size_t aligned = (sz + 15) & ~(size_t)15;
    if (l_tt__pool_used + aligned > l_tt__pool_cap) return 0;
    void *p = l_tt__pool_base + l_tt__pool_used;
    l_tt__pool_used += aligned;
    return p;
}

static inline void l_tt_free(void *ptr) {
    (void)ptr; /* bump allocator — frees are no-ops; use watermark restore */
}

/* Watermark API: use to reclaim a burst of allocations (e.g. for one glyph).
   Call l_tt__save() before, l_tt__restore() after copying the result out. */
static inline size_t l_tt__save(void)        { return l_tt__pool_used; }
static inline void   l_tt__restore(size_t m) { l_tt__pool_used = m;    }

// -- Configure stb_truetype for freestanding ----------------------------------
// Override every STBTT_* hook so stb_truetype does NOT include <math.h>,
// <stdlib.h>, <string.h>, or <assert.h>. All math uses l_os.h doubles.

/* Use STBTT_STATIC so unused functions can be eliminated by --gc-sections. */
#define STBTT_STATIC

#define STBTT_ifloor(x)          ((int)l_floor(x))
#define STBTT_iceil(x)           ((int)l_ceil(x))
#define STBTT_sqrt(x)            l_sqrt(x)
#define STBTT_pow(x, y)          l_pow((x), (y))
#define STBTT_fmod(x, y)         l_fmod((x), (y))
#define STBTT_cos(x)             l_cos(x)
#define STBTT_acos(x)            l_acos(x)
#define STBTT_fabs(x)            l_fabs(x)

/* userdata argument is unused; allocator is process-global. */
#define STBTT_malloc(x, u)       ((void)(u), l_tt_malloc((size_t)(x)))
#define STBTT_free(x, u)         ((void)(u), l_tt_free(x))

#define STBTT_assert(x)          ((void)0)

#define STBTT_strlen(x)          l_strlen(x)
#define STBTT_memcpy             l_memcpy
#define STBTT_memset             l_memset

/* Suppress compiler warnings from stb_truetype's code */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#pragma clang diagnostic ignored "-Wcast-qual"
#endif
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#pragma GCC diagnostic ignored "-Wcast-qual"
#endif
#endif

/* Temporarily undefine l_os.h macros that could clash with names used by
   stb_truetype (or by its internal <math.h>/<string.h> guarded includes). */
#ifdef rand
#undef rand
#endif
#ifdef setenv
#undef setenv
#endif
#ifdef strerror
#undef strerror
#endif

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/* Restore l_os.h macros */
#ifndef rand
#define rand l_rand
#endif
#ifndef setenv
#define setenv l_setenv
#endif
#ifndef strerror
#define strerror l_strerror
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
#endif

// -- Public API implementation ------------------------------------------------

static inline int l_tt_init_font(const unsigned char *data, int len,
                                  struct stbtt_fontinfo *info) {
    if (!data || len <= 0 || !info) return 0;
    int offset = stbtt_GetFontOffsetForIndex(data, 0);
    if (offset < 0) return 0;
    return stbtt_InitFont(info, data, offset);
}

static inline float l_tt_scale_for_pixel_height(const struct stbtt_fontinfo *info,
                                                 float pixels) {
    return stbtt_ScaleForPixelHeight(info, pixels);
}

static inline void l_tt_vmetrics(const struct stbtt_fontinfo *info,
                                  int *ascent, int *descent, int *lineGap) {
    stbtt_GetFontVMetrics(info, ascent, descent, lineGap);
}

static inline void l_tt_hmetrics(const struct stbtt_fontinfo *info, int codepoint,
                                  int *advance, int *lsb) {
    stbtt_GetCodepointHMetrics(info, codepoint, advance, lsb);
}

static inline int l_tt_kern_advance(const struct stbtt_fontinfo *info,
                                     int cp1, int cp2) {
    return stbtt_GetCodepointKernAdvance(info, cp1, cp2);
}

/// Rasterize a glyph into a freshly mmap'd 1-channel alpha bitmap.
/// Returns NULL on failure. Uses watermark restore so the internal pool is
/// reclaimed after copying the result, making repeated calls safe.
static inline unsigned char *l_tt_render_glyph(const struct stbtt_fontinfo *info,
                                                int codepoint, float scale,
                                                int *w, int *h,
                                                int *xoff, int *yoff) {
    if (w)    *w    = 0;
    if (h)    *h    = 0;
    if (xoff) *xoff = 0;
    if (yoff) *yoff = 0;
    if (!info || !w || !h) return 0;

    size_t mark = l_tt__save();

    int bw = 0, bh = 0, bxo = 0, byo = 0;
    unsigned char *tmp = stbtt_GetCodepointBitmap(info, scale, scale, codepoint,
                                                   &bw, &bh, &bxo, &byo);
    if (!tmp || bw <= 0 || bh <= 0) {
        l_tt__restore(mark);
        return 0;
    }

    size_t bsz = (size_t)bw * (size_t)bh;
    unsigned char *result = (unsigned char *)l_mmap(0, bsz,
                                                    L_PROT_READ | L_PROT_WRITE,
                                                    L_MAP_PRIVATE | L_MAP_ANONYMOUS,
                                                    -1, 0);
    if (result == (unsigned char *)L_MAP_FAILED) {
        l_tt__restore(mark);
        return 0;
    }
    l_memcpy(result, tmp, bsz);
    l_tt__restore(mark);

    *w = bw;
    *h = bh;
    if (xoff) *xoff = bxo;
    if (yoff) *yoff = byo;
    return result;
}

static inline void l_tt_free_bitmap(unsigned char *bitmap, int w, int h) {
    if (bitmap && bitmap != (unsigned char *)L_MAP_FAILED) {
        l_munmap(bitmap, (size_t)w * (size_t)h);
    }
}

/// Blit an alpha bitmap into a 32-bit ARGB canvas using the given color.
/// Uses straight alpha compositing with a gamma ≈ 2 stem-darkening
/// approximation (a' = 1 - (1-a)^2) applied to the source bitmap alpha.
/// This counteracts the sRGB-space compositing that otherwise makes
/// anti-aliased text look too thin / surrounded by dark halos, especially
/// for light-on-dark rendering. `color` is used for its RGB channels; the
/// color's alpha is multiplied with the (boosted) bitmap alpha.
/// Clips to (dw, dh).
static inline void l_tt_blit_alpha(uint32_t *dst, int dw, int dh, int dstride,
                                    int x, int y,
                                    const unsigned char *alpha, int aw, int ah,
                                    uint32_t color) {
    if (!dst || !alpha || dw <= 0 || dh <= 0 || aw <= 0 || ah <= 0) return;
    int pitch = dstride > 0 ? dstride / 4 : dw;

    unsigned cR = (color >> 16) & 0xFF;
    unsigned cG = (color >>  8) & 0xFF;
    unsigned cB =  color        & 0xFF;
    unsigned cA = (color >> 24) & 0xFF;

    for (int j = 0; j < ah; j++) {
        int dy = y + j;
        if (dy < 0 || dy >= dh) continue;
        for (int i = 0; i < aw; i++) {
            int dx = x + i;
            if (dx < 0 || dx >= dw) continue;
            unsigned a = alpha[j * aw + i];
            if (a == 0) continue;
            /* Gamma ≈ 2 correction (cheap stem-darkening). */
            unsigned inv_g = 255u - a;
            a = 255u - (inv_g * inv_g) / 255u;
            unsigned src_a = (a * cA) / 255;
            if (src_a == 0) continue;
            uint32_t *p = &dst[dy * pitch + dx];
            uint32_t d = *p;
            unsigned dR = (d >> 16) & 0xFF;
            unsigned dG = (d >>  8) & 0xFF;
            unsigned dB =  d        & 0xFF;
            unsigned dA = (d >> 24) & 0xFF;
            unsigned inv = 255 - src_a;
            unsigned oR = (cR * src_a + dR * inv) / 255;
            unsigned oG = (cG * src_a + dG * inv) / 255;
            unsigned oB = (cB * src_a + dB * inv) / 255;
            unsigned oA = src_a + (dA * inv) / 255;
            if (oA > 255) oA = 255;
            *p = ((uint32_t)oA << 24) | (oR << 16) | (oG << 8) | oB;
        }
    }
}

/// Draw a UTF-8 string onto a 32-bit ARGB canvas using subpixel positioning
/// and 2×2 supersampled rasterization. Internally this:
///   * carries the pen in float so each glyph is rasterized at its true
///     sub-pixel horizontal offset;
///   * renders each glyph at 2× the target resolution (OX=OY=2) and
///     box-downsamples, which preserves subpixel anti-aliasing and
///     eliminates the integer-pixel snap you get from the raw
///     `l_tt_render_glyph` path;
///   * applies kerning between successive code-points;
///   * composites with the gamma-approximated `l_tt_blit_alpha`.
/// Non-ASCII bytes are treated as opaque glyph indices (passed as-is to
/// stb_truetype's codepoint API), which is correct for the ASCII subset
/// used by the built-in samples; a future UTF-8 decoder can be plugged in
/// here without changing the signature. Returns the pen x after the final
/// glyph (useful for chaining calls).
static inline float l_tt_draw_text(uint32_t *dst, int dw, int dh, int dstride,
                                   const struct stbtt_fontinfo *info,
                                   float x, float baseline_y,
                                   float pixel_height,
                                   const char *utf8, uint32_t color) {
    if (!dst || !info || !utf8 || dw <= 0 || dh <= 0) return x;
    float scale = stbtt_ScaleForPixelHeight(info, pixel_height);
    if (scale <= 0.0f) return x;

    const int OX = 2, OY = 2;
    float pen = x;
    int baseline_i = (int)l_floor((double)baseline_y);
    int prev_cp = 0;

    for (const char *p = utf8; *p; p++) {
        int cp = (unsigned char)*p;

        if (prev_cp) {
            int kern = stbtt_GetCodepointKernAdvance(info, prev_cp, cp);
            pen += (float)kern * scale;
        }

        float pf = (float)l_floor((double)pen);
        float shift_x = pen - pf;
        int pen_i = (int)pf;

        /* Screen-space bbox (for placement) */
        int sx0 = 0, sy0 = 0, sx1 = 0, sy1 = 0;
        stbtt_GetCodepointBitmapBoxSubpixel(info, cp, scale, scale,
            shift_x, 0.0f, &sx0, &sy0, &sx1, &sy1);

        /* Oversampled bbox (for rasterization) */
        int ox0 = 0, oy0 = 0, ox1 = 0, oy1 = 0;
        stbtt_GetCodepointBitmapBoxSubpixel(info, cp,
            scale * (float)OX, scale * (float)OY,
            shift_x * (float)OX, 0.0f,
            &ox0, &oy0, &ox1, &oy1);
        int bw_over = ox1 - ox0;
        int bh_over = oy1 - oy0;

        if (bw_over > 0 && bh_over > 0) {
            size_t mark = l_tt__save();
            unsigned char *tmp = (unsigned char *)l_tt_malloc(
                (size_t)bw_over * (size_t)bh_over);
            if (tmp) {
                l_memset(tmp, 0, (size_t)bw_over * (size_t)bh_over);
                stbtt_MakeCodepointBitmapSubpixel(info, tmp,
                    bw_over, bh_over, bw_over,
                    scale * (float)OX, scale * (float)OY,
                    shift_x * (float)OX, 0.0f, cp);

                /* Downsample OX×OY → 1 screen pixel by box average. */
                int sw = bw_over / OX;
                int sh = bh_over / OY;
                if (sw > 0 && sh > 0) {
                    unsigned char *ds = (unsigned char *)l_tt_malloc(
                        (size_t)sw * (size_t)sh);
                    if (ds) {
                        unsigned div = (unsigned)(OX * OY);
                        for (int j = 0; j < sh; j++) {
                            for (int i = 0; i < sw; i++) {
                                unsigned sum = 0;
                                for (int oy = 0; oy < OY; oy++)
                                    for (int ox = 0; ox < OX; ox++)
                                        sum += tmp[(j * OY + oy) * bw_over
                                                   + (i * OX + ox)];
                                ds[j * sw + i] = (unsigned char)(sum / div);
                            }
                        }
                        l_tt_blit_alpha(dst, dw, dh, dstride,
                                        pen_i + sx0, baseline_i + sy0,
                                        ds, sw, sh, color);
                    }
                }
            }
            l_tt__restore(mark);
        }

        int adv = 0, lsb = 0;
        stbtt_GetCodepointHMetrics(info, cp, &adv, &lsb);
        pen += (float)adv * scale;
        prev_cp = cp;
    }
    return pen;
}

#endif /* L_WITHDEFS */
#endif /* L_TT_H */
