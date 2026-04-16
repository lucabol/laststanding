// l_svg.h - Freestanding SVG rasterization for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_svg.h"   // pulls in l_os.h + vendored NanoSVG fork
//
// Wraps a vendored NanoSVG fork for freestanding use (no libc, no malloc,
// no stdio). Parses and rasterizes an SVG subset from memory buffers to
// ARGB pixels. Uses mmap-backed allocation (no heap).
//
// Supported elements: svg, g, path, rect, circle, ellipse, line, polyline,
// polygon, defs, linearGradient, radialGradient.
// Not supported: text, image, symbol, use, clipPath, mask, filter.
//
// API:
//   l_svg_load_mem(data, len, opt, &w, &h)  -> uint32_t* ARGB pixel buffer (or NULL)
//   l_svg_free_pixels(pixels, w, h)         -> release the pixel buffer

#ifndef L_SVG_H
#define L_SVG_H

#include "l_os.h"

// -- Public API ---------------------------------------------------------------

typedef struct {
    int width;        // requested raster width (0 = use intrinsic/viewBox)
    int height;       // requested raster height (0 = use intrinsic/viewBox)
    float dpi;        // rendering DPI (0 = default 96.0f)
} L_SvgOptions;

/// Rasterize SVG from a memory buffer. Returns ARGB pixel data or NULL.
/// Writes image dimensions to *out_w and *out_h.
/// opt may be NULL (all defaults: intrinsic size, 96 DPI).
/// Call l_svg_free_pixels() to release.
static inline uint32_t *l_svg_load_mem(const unsigned char *data, int len,
                                        const L_SvgOptions *opt,
                                        int *out_w, int *out_h);

/// Free pixel data returned by l_svg_load_mem(). w and h must match the decode.
static inline void l_svg_free_pixels(uint32_t *pixels, int w, int h);

// -- Implementation (only included once with L_WITHDEFS) ----------------------

#ifdef L_WITHDEFS

// -- Freestanding allocator for NanoSVG ---------------------------------------
// NanoSVG needs malloc/realloc/free. We provide a bump allocator backed by a
// single large mmap region (same design as l_img.h). This pool is independent
// of the l_img.h pool. Reset after each decode.

#define L_SVG_POOL_SIZE (256 * 1024 * 1024)

static unsigned char *l_svg__pool_base;
static size_t l_svg__pool_used;
static size_t l_svg__pool_cap;

#define L_SVG_MAX_ALLOCS 4096
static struct { void *ptr; size_t size; } l_svg__allocs[L_SVG_MAX_ALLOCS];
static int l_svg__alloc_count;

static inline void l_svg__pool_init(void) {
    if (l_svg__pool_base) return;
    l_svg__pool_base = (unsigned char *)l_mmap(0, L_SVG_POOL_SIZE,
                                               L_PROT_READ | L_PROT_WRITE,
                                               L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (l_svg__pool_base == (unsigned char *)L_MAP_FAILED) l_svg__pool_base = 0;
    l_svg__pool_cap = L_SVG_POOL_SIZE;
    l_svg__pool_used = 0;
    l_svg__alloc_count = 0;
}

static inline void *l_svg_malloc(size_t sz) {
    l_svg__pool_init();
    if (!l_svg__pool_base || sz == 0) return 0;
    size_t aligned = (sz + 15) & ~(size_t)15;
    if (l_svg__pool_used + aligned > l_svg__pool_cap) return 0;
    void *p = l_svg__pool_base + l_svg__pool_used;
    l_svg__pool_used += aligned;
    if (l_svg__alloc_count < L_SVG_MAX_ALLOCS) {
        l_svg__allocs[l_svg__alloc_count].ptr = p;
        l_svg__allocs[l_svg__alloc_count].size = sz;
        l_svg__alloc_count++;
    }
    return p;
}

static inline void l_svg_free(void *ptr) {
    (void)ptr; /* bump allocator - pool is reset after each decode */
}

static inline void *l_svg_realloc(void *ptr, size_t newsz) {
    if (!ptr) return l_svg_malloc(newsz);
    size_t oldsz = 0;
    for (int i = 0; i < l_svg__alloc_count; i++) {
        if (l_svg__allocs[i].ptr == ptr) { oldsz = l_svg__allocs[i].size; break; }
    }
    /* Try to extend in-place if ptr is at the tip of the pool */
    size_t old_aligned = (oldsz + 15) & ~(size_t)15;
    if ((unsigned char *)ptr + old_aligned == l_svg__pool_base + l_svg__pool_used) {
        size_t new_aligned = (newsz + 15) & ~(size_t)15;
        if (new_aligned > old_aligned) {
            size_t extra = new_aligned - old_aligned;
            if (l_svg__pool_used + extra <= l_svg__pool_cap) {
                l_svg__pool_used += extra;
                for (int i = 0; i < l_svg__alloc_count; i++) {
                    if (l_svg__allocs[i].ptr == ptr) { l_svg__allocs[i].size = newsz; break; }
                }
                return ptr;
            }
        } else {
            /* Shrinking - just update the size record */
            for (int i = 0; i < l_svg__alloc_count; i++) {
                if (l_svg__allocs[i].ptr == ptr) { l_svg__allocs[i].size = newsz; break; }
            }
            return ptr;
        }
    }
    /* Otherwise allocate new and copy */
    void *newp = l_svg_malloc(newsz);
    if (!newp) return 0;
    size_t copy = oldsz < newsz ? oldsz : newsz;
    if (copy > 0) l_memcpy(newp, ptr, copy);
    return newp;
}

static inline void l_svg__pool_reset(void) {
    l_svg__pool_used = 0;
    l_svg__alloc_count = 0;
}

// -- Configure NanoSVG for freestanding ---------------------------------------

#define NSVG_MALLOC(sz)        l_svg_malloc(sz)
#define NSVG_REALLOC(ptr, sz)  l_svg_realloc((ptr), (sz))
#define NSVG_FREE(ptr)         l_svg_free(ptr)

// -- Float math shims (sinf, cosf, etc.) --------------------------------------
#include "compat/math.h"

/* Suppress compiler warnings from NanoSVG code */
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wsign-compare"
#pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
#endif

#define NANOSVG_IMPLEMENTATION
#include "compat/nanosvg/nanosvg.h"

#define NANOSVGRAST_IMPLEMENTATION
#include "compat/nanosvg/nanosvgrast.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
#endif

// -- Public API implementation ------------------------------------------------

static inline uint32_t *l_svg_load_mem(const unsigned char *data, int len,
                                        const L_SvgOptions *opt,
                                        int *out_w, int *out_h) {
    if (!data || len <= 0 || !out_w || !out_h) return 0;

    float dpi = 96.0f;
    int req_w = 0, req_h = 0;
    if (opt) {
        if (opt->dpi > 0.0f) dpi = opt->dpi;
        req_w = opt->width;
        req_h = opt->height;
    }

    /* nsvgParse requires a mutable NUL-terminated string.
       Allocate from pool, copy, and NUL-terminate. */
    char *buf = (char *)l_svg_malloc((size_t)len + 1);
    if (!buf) return 0;
    l_memcpy(buf, data, (size_t)len);
    buf[len] = '\0';

    /* Parse SVG */
    NSVGimage *image = nsvgParse(buf, "px", dpi);
    if (!image) { l_svg__pool_reset(); return 0; }

    /* Resolve output dimensions */
    float svg_w = image->width;
    float svg_h = image->height;
    if (svg_w <= 0.0f || svg_h <= 0.0f) {
        if (req_w <= 0 || req_h <= 0) {
            nsvgDelete(image);
            l_svg__pool_reset();
            return 0;
        }
        svg_w = (float)req_w;
        svg_h = (float)req_h;
    }

    int w, h;
    float scale;
    if (req_w > 0 && req_h > 0) {
        w = req_w;
        h = req_h;
        float sx = (float)w / svg_w;
        float sy = (float)h / svg_h;
        scale = sx < sy ? sx : sy;
    } else if (req_w > 0) {
        w = req_w;
        scale = (float)w / svg_w;
        h = (int)(svg_h * scale + 0.5f);
        if (h <= 0) h = 1;
    } else if (req_h > 0) {
        h = req_h;
        scale = (float)h / svg_h;
        w = (int)(svg_w * scale + 0.5f);
        if (w <= 0) w = 1;
    } else {
        w = (int)(svg_w + 0.5f);
        h = (int)(svg_h + 0.5f);
        if (w <= 0) w = 1;
        if (h <= 0) h = 1;
        scale = (float)w / svg_w;
    }

    if (w <= 0 || h <= 0 || w > 65536 || h > 65536) {
        nsvgDelete(image);
        l_svg__pool_reset();
        return 0;
    }

    /* Rasterize to RGBA */
    NSVGrasterizer *rast = nsvgCreateRasterizer();
    if (!rast) {
        nsvgDelete(image);
        l_svg__pool_reset();
        return 0;
    }

    unsigned char *rgba = (unsigned char *)l_svg_malloc((size_t)(w * h * 4));
    if (!rgba) {
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        l_svg__pool_reset();
        return 0;
    }

    nsvgRasterize(rast, image, 0, 0, scale, rgba, w, h, w * 4);

    /* Convert RGBA -> ARGB in-place (NanoSVG outputs R,G,B,A byte order) */
    int n = w * h;
    uint32_t *pixels = (uint32_t *)rgba;
    for (int i = 0; i < n; i++) {
        unsigned char r = rgba[i * 4 + 0];
        unsigned char g = rgba[i * 4 + 1];
        unsigned char b = rgba[i * 4 + 2];
        unsigned char a = rgba[i * 4 + 3];
        pixels[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) |
                     ((uint32_t)g << 8) | (uint32_t)b;
    }

    /* Copy result to a dedicated mmap region so we can reset the pool */
    size_t pixsz = (size_t)(n * 4);
    uint32_t *result = (uint32_t *)l_mmap(0, pixsz, L_PROT_READ | L_PROT_WRITE,
                                           L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (result == (uint32_t *)L_MAP_FAILED) {
        nsvgDeleteRasterizer(rast);
        nsvgDelete(image);
        l_svg__pool_reset();
        return 0;
    }
    l_memcpy(result, pixels, pixsz);

    /* Cleanup - all pool allocations freed by reset */
    nsvgDeleteRasterizer(rast);
    nsvgDelete(image);
    l_svg__pool_reset();

    *out_w = w;
    *out_h = h;
    return result;
}

static inline void l_svg_free_pixels(uint32_t *pixels, int w, int h) {
    if (pixels && pixels != (uint32_t *)L_MAP_FAILED) {
        l_munmap(pixels, (size_t)(w * h * 4));
    }
}

#endif /* L_WITHDEFS */
#endif /* L_SVG_H */
