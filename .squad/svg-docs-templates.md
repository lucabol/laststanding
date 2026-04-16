# SVG Documentation: Templates for README

This document shows the exact sections that will be added to README.md once `l_svg.h` lands. Use these as a checklist and reference.

---

## 1. Header Table Update (Line ~5)

**Location:** Right after the opening intro, in the feature table.

**Current:**
```markdown
| Header | What it provides |
|--------|-----------------|
| `l_os.h` | String/memory functions, file I/O, processes, pipes, terminal control, environment access, hash maps, SHA-256, glob matching, time formatting |
| `l_gfx.h` | Pixel graphics — drawing primitives, scaled bitmap font, pixel blitting, alpha blending, keyboard/mouse input, clipboard, and window/framebuffer/terminal backends |
| `l_img.h` | Image decoding — PNG, JPEG, BMP, GIF, TGA from memory buffers via vendored stb_image (freestanding, no libc) |
| `l_tls.h` | TLS/HTTPS client — SChannel on Windows, BearSSL on Linux (zero deps on both). Up to 8 simultaneous connections |
| `l_ui.h` | Immediate-mode UI — buttons, checkboxes, sliders, text inputs, layout helpers (built on `l_gfx.h`) |
```

**Updated:**
```markdown
| Header | What it provides |
|--------|-----------------|
| `l_os.h` | String/memory functions, file I/O, processes, pipes, terminal control, environment access, hash maps, SHA-256, glob matching, time formatting |
| `l_gfx.h` | Pixel graphics — drawing primitives, scaled bitmap font, pixel blitting, alpha blending, keyboard/mouse input, clipboard, and window/framebuffer/terminal backends |
| `l_img.h` | Image decoding — PNG, JPEG, BMP, GIF, TGA from memory buffers via vendored stb_image (freestanding, no libc) |
| `l_svg.h` | **SVG rasterization — icons, diagrams, and simple vector assets from memory buffers; subset renderer (no text, image, or clipPath); ARGB output compatible with `l_gfx.h` blitting** |
| `l_tls.h` | TLS/HTTPS client — SChannel on Windows, BearSSL on Linux (zero deps on both). Up to 8 simultaneous connections |
| `l_ui.h` | Immediate-mode UI — buttons, checkboxes, sliders, text inputs, layout helpers (built on `l_gfx.h`) |
```

---

## 2. Quick-Start SVG Example (After line ~124, following Image Decoding example)

**Location:** In the "Quick Start" section, right after the `l_img.h` example and before the `l_tls.h` example.

**Template:**
```markdown
### SVG Rasterization (`l_svg.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"
#include "l_svg.h"       // pulls in l_os.h (freestanding, no libc)

int main(int argc, char *argv[]) {
    l_getenv_init(argc, argv);

    if (argc < 2) {
        l_puts("Usage: svg_view <svg_file> [width] [height]\n");
        return 0;
    }

    // Read SVG file into memory
    L_FD fd = l_open(argv[1], 0, 0);
    if (fd < 0) { l_puts("Error: cannot open file\n"); return 1; }
    
    long long sz = l_lseek(fd, 0, 2); l_lseek(fd, 0, 0);
    if (sz <= 0 || sz > 8 * 1024 * 1024) {
        l_puts("Error: file too large or empty (max 8MB)\n");
        l_close(fd);
        return 1;
    }
    
    unsigned char *buf = (unsigned char *)l_mmap(0, (size_t)sz,
        L_PROT_READ, L_MAP_PRIVATE, fd, 0);
    l_close(fd);
    if (buf == (unsigned char *)L_MAP_FAILED) {
        l_puts("Error: cannot read file\n");
        return 1;
    }

    // Rasterize SVG at specified size (or intrinsic if width/height are 0)
    int w = argc > 2 ? l_atoi(argv[2]) : 512;
    int h = argc > 3 ? l_atoi(argv[3]) : 512;
    
    L_SvgOptions opt = { .width = w, .height = h, .dpi = 96.0f, .id = NULL };
    int out_w = 0, out_h = 0;
    uint32_t *pixels = l_svg_load_mem(buf, (int)sz, &opt, &out_w, &out_h);
    l_munmap(buf, (size_t)sz);

    if (!pixels) {
        l_puts("Error: unsupported or corrupt SVG\n");
        return 1;
    }

    // Display in a window
    L_Canvas c;
    l_canvas_open(&c, out_w, out_h, argv[1]);
    l_blit(&c, 0, 0, out_w, out_h, pixels, out_w * 4);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27)
        l_sleep_ms(16);

    l_canvas_close(&c);
    l_svg_free_pixels(pixels, out_w, out_h);
    return 0;
}
```
```

---

## 3. Function Reference Section for `l_svg.h` (After line ~901, following `l_img.h` reference)

**Location:** Between `## Function Reference — l_img.h` and `## Function Reference — l_gfx.h`, or inserted just after `l_img.h` reference.

**Template:**
```markdown
## Function Reference — `l_svg.h`

Freestanding SVG rasterization powered by a vendored NanoSVG-derived fork. Parses SVG from memory buffers and rasterizes to ARGB pixels. Supports a subset of SVG (paths, basic shapes, gradients, transforms); does not support text, embedded images, clipping, or advanced features. Uses the same 256 MB demand-paged bump allocator as `l_img.h`.

**Supported elements:** `svg`, `g`, `path`, `rect`, `circle`, `ellipse`, `line`, `polyline`, `polygon`, `defs`, basic `<linearGradient>` and `<radialGradient>` with stops.

**Unsupported elements:** `text`, `image`, `symbol`, `use`, `clipPath`, `mask`, `filter`, CSS stylesheets, advanced color spaces, ICC profiles.

| Function | Description |
|----------|-------------|
| `l_svg_load_mem(data, len, opt, out_w, out_h)` | Rasterizes an SVG from memory. `opt` specifies target width/height (0 = use intrinsic), DPI (default 96), and optional element ID. Returns ARGB pixel buffer or NULL on error. Sets `*out_w` and `*out_h` to actual raster dimensions. |
| `l_svg_free_pixels(pixels, w, h)` | Releases memory used by a previously rasterized SVG. |

**Type:** `L_SvgOptions`

| Field | Type | Purpose |
|-------|------|---------|
| `width` | `int` | Target raster width (pixels). 0 = use intrinsic or fail. |
| `height` | `int` | Target raster height (pixels). 0 = use intrinsic or fail. |
| `dpi` | `float` | Resolution multiplier. Default 96.0f (1x scale). |
| `id` | `const char*` | Optional SVG element ID or fragment identifier. NULL = whole document. |

**Sizing behavior:**
- If both `width` and `height` are 0, uses the SVG's intrinsic size (from `viewBox` or `width`/`height` attributes). If the document has no intrinsic size, returns NULL.
- If one dimension is 0 and the other is non-zero, scales preserving aspect ratio.
- If both are non-zero, rasterizes at that size (may distort aspect ratio).

**Example:**
```c
L_SvgOptions opt = { 512, 512, 96.0f, NULL };  // 512x512 at 96 DPI, whole document
uint32_t *pixels = l_svg_load_mem(buf, buflen, &opt, &w, &h);
if (pixels) {
    // Use pixels...
    l_svg_free_pixels(pixels, w, h);
}
```
```

---

## 4. Compiler Flags Addition (Line ~500, in the L_Defs section)

**Optional:** If NanoSVG fork requires a special flag for float-math shims.

**Addition to table:**
```markdown
| `L_WITHFLOATMATH` | (internal use by `l_svg.h`) Enables float-math entry points (`sinf`, `cosf`, etc.) for SVG rasterization. Automatically defined by `l_svg.h` if needed. |
```

**Or, simpler prose note:**
```markdown
`l_svg.h` automatically provides float-math entry points (`sinf`, `cosf`, `powf`, `atan2f`, `sqrtf`, `ceilf`, `floorf`, `fmodf`) via `l_os.h`, so no user action is required.
```

---

## 5. Key Types Update (Line ~515, optional)

**Addition to the types table:**
```markdown
| `L_SvgOptions` | SVG rasterization options — width/height (0 for intrinsic), DPI (default 96), optional element ID. |
```

---

## Notes for Parker (Engine) & Dallas (Testing)

- These templates assume the public API is stable (signature of `l_svg_load_mem`, `l_svg_free_pixels`, and `L_SvgOptions` struct).
- If the API changes before landing, these templates need updates.
- Once `l_svg.h` lands, run `.\gen-docs.ps1` to auto-generate the function reference tables (if the script has been updated to scan `l_svg.h`).
- The `svg_view.c` example should be added to `examples/` directory so it shows up in the build.

---

## Validation Checklist

Before committing docs updates:

- [ ] Header table row added and formatted consistently
- [ ] Quick-start SVG example added (position: after `l_img.h` example, before `l_tls.h`)
- [ ] Function reference section added (with type documentation if needed)
- [ ] `svg_view.c` example added to `examples/` directory
- [ ] `gen-docs.ps1` updated to parse `l_svg.h` doc-comments (if not already done)
- [ ] Run `.\gen-docs.ps1` to regenerate and verify no errors
- [ ] Spot-check that README renders correctly in GitHub web UI
- [ ] Confirm all supported/unsupported elements are accurately listed

---

**Template created by:** Brett (Docs)  
**For use after:** `l_svg.h` implementation lands  
**Superseded by:** Any explicit API design doc from Parker (Engine)
