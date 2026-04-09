// l_img.h — Freestanding image decoding for laststanding
//
// Usage: #define L_MAINFILE
//        #include "l_img.h"   // pulls in l_os.h + stb_image.h
//
// Wraps stb_image.h for freestanding use (no libc, no malloc, no stdio).
// Decodes PNG, JPEG, BMP, GIF, TGA from memory buffers.
// Uses mmap-backed allocation (no heap).
//
// API:
//   l_img_load_mem(data, len, &w, &h)  → uint32_t* ARGB pixel buffer (or NULL)
//   l_img_free(pixels, w, h)           → release the pixel buffer

#ifndef L_IMG_H
#define L_IMG_H

#include "l_os.h"

// ── Freestanding allocator for stb_image ─────────────────────────────────────
// stb_image needs malloc/realloc/free. We provide a simple bump allocator
// backed by a single large mmap region. This is sufficient for decoding
// one image at a time (reset after each decode).

#ifdef L_WITHDEFS

/* Pool size: 256 MB virtual. On modern OSes with MAP_ANONYMOUS, pages are
   demand-paged — only physical memory for pages actually touched is used.
   A 256 MB reservation costs ~0 until pixels are decoded into it. */
#define L_IMG_POOL_SIZE (256 * 1024 * 1024)

/* Pool state — initialized on first use */
static unsigned char *l_img__pool_base;
static size_t l_img__pool_used;
static size_t l_img__pool_cap;

/* Track allocations for realloc */
#define L_IMG_MAX_ALLOCS 1024
static struct { void *ptr; size_t size; } l_img__allocs[L_IMG_MAX_ALLOCS];
static int l_img__alloc_count;

static inline void l_img__pool_init(void) {
    if (l_img__pool_base) return;
    l_img__pool_base = (unsigned char *)l_mmap(0, L_IMG_POOL_SIZE,
                                                L_PROT_READ | L_PROT_WRITE,
                                                L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (l_img__pool_base == (unsigned char *)L_MAP_FAILED) l_img__pool_base = 0;
    l_img__pool_cap = L_IMG_POOL_SIZE;
    l_img__pool_used = 0;
    l_img__alloc_count = 0;
}

static inline void *l_img_malloc(size_t sz) {
    l_img__pool_init();
    if (!l_img__pool_base) return 0;
    size_t aligned = (sz + 15) & ~(size_t)15;
    if (l_img__pool_used + aligned > l_img__pool_cap) return 0;
    void *p = l_img__pool_base + l_img__pool_used;
    l_img__pool_used += aligned;
    if (l_img__alloc_count < L_IMG_MAX_ALLOCS) {
        l_img__allocs[l_img__alloc_count].ptr = p;
        l_img__allocs[l_img__alloc_count].size = sz;
        l_img__alloc_count++;
    }
    return p;
}

static inline void l_img_free(void *ptr) {
    /* Bump allocator — individual frees are no-ops.
       The pool is reset after each image decode via l_img__pool_reset(). */
    (void)ptr;
}

static inline void *l_img_realloc(void *ptr, size_t newsz) {
    if (!ptr) return l_img_malloc(newsz);
    /* Find old size */
    size_t oldsz = 0;
    for (int i = 0; i < l_img__alloc_count; i++) {
        if (l_img__allocs[i].ptr == ptr) { oldsz = l_img__allocs[i].size; break; }
    }
    /* If ptr is at the tip of the pool, try to extend in-place */
    size_t old_aligned = (oldsz + 15) & ~(size_t)15;
    if ((unsigned char *)ptr + old_aligned == l_img__pool_base + l_img__pool_used) {
        size_t new_aligned = (newsz + 15) & ~(size_t)15;
        size_t extra = new_aligned - old_aligned;
        if (l_img__pool_used + extra <= l_img__pool_cap) {
            l_img__pool_used += extra;
            for (int i = 0; i < l_img__alloc_count; i++) {
                if (l_img__allocs[i].ptr == ptr) { l_img__allocs[i].size = newsz; break; }
            }
            return ptr;
        }
    }
    /* Otherwise allocate new and copy */
    void *newp = l_img_malloc(newsz);
    if (!newp) return 0;
    size_t copy = oldsz < newsz ? oldsz : newsz;
    if (copy > 0) l_memcpy(newp, ptr, copy);
    return newp;
}

static inline void l_img__pool_reset(void) {
    l_img__pool_used = 0;
    l_img__alloc_count = 0;
}

// ── Configure stb_image for freestanding ─────────────────────────────────────

#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_PSD
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_NO_THREAD_LOCALS
#define STBI_ASSERT(x)                    ((void)0)
#define STBI_MALLOC(sz)                   l_img_malloc(sz)
#define STBI_REALLOC(p, newsz)            l_img_realloc(p, newsz)
#define STBI_FREE(p)                      l_img_free(p)
#define STBI_NO_SIMD

/* Provide string.h functions that stb_image needs via l_os.h equivalents */
#define stbi__memcpy  l_memcpy
#define stbi__memset  l_memset
#define stbi__memmove l_memmove

/* stb_image may call memcmp/strlen/abs directly — redirect to l_os.h */
#ifndef memcmp
#define memcmp l_memcmp
#endif
#ifndef strlen
#define strlen l_strlen
#endif
#ifndef strcmp
#define strcmp l_strcmp
#endif

/* Suppress compiler warnings from stb_image's code */
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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
#endif

// ── Public API ───────────────────────────────────────────────────────────────

/// Decode an image from a memory buffer. Returns ARGB pixel data or NULL.
/// Writes image dimensions to *w and *h. Call l_img_free_pixels() to release.
static inline uint32_t *l_img_load_mem(const unsigned char *data, int len,
                                        int *w, int *h) {
    if (!data || len <= 0) return 0;
    int comp;
    /* Request 4 channels (RGBA) */
    unsigned char *rgba = stbi_load_from_memory(data, len, w, h, &comp, 4);
    if (!rgba) { l_img__pool_reset(); return 0; }

    /* Convert RGBA → ARGB in-place (stb gives R,G,B,A byte order) */
    int n = (*w) * (*h);
    uint32_t *pixels = (uint32_t *)rgba;
    for (int i = 0; i < n; i++) {
        unsigned char r = rgba[i * 4 + 0];
        unsigned char g = rgba[i * 4 + 1];
        unsigned char b = rgba[i * 4 + 2];
        unsigned char a = rgba[i * 4 + 3];
        pixels[i] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
    }

    /* Copy result to a dedicated mmap region so we can reset the pool */
    size_t pixsz = (size_t)(n * 4);
    uint32_t *result = (uint32_t *)l_mmap(0, pixsz, L_PROT_READ | L_PROT_WRITE,
                                           L_MAP_PRIVATE | L_MAP_ANONYMOUS, -1, 0);
    if (result == (uint32_t *)L_MAP_FAILED) { l_img__pool_reset(); return 0; }
    l_memcpy(result, pixels, pixsz);
    l_img__pool_reset();
    return result;
}

/// Free pixel data returned by l_img_load_mem(). w and h must match the decode.
static inline void l_img_free_pixels(uint32_t *pixels, int w, int h) {
    if (pixels && pixels != (uint32_t *)L_MAP_FAILED) {
        l_munmap(pixels, (size_t)(w * h * 4));
    }
}

#endif /* L_WITHDEFS */
#endif /* L_IMG_H */
