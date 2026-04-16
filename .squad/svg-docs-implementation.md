# SVG Documentation: Implementation Complete

**Status:** Landed  
**Implemented by:** Brett (Docs)  
**Scope:** README.md, gen-docs.ps1, examples/svg_view.c  

## Summary of Changes

### 1. README.md Header Table Update (Line 3–12)

- Updated opening: "Five header files" → "Six header files"
- Added `l_svg.h` row to feature table with clear subset limitations

**Example entry:**
```markdown
| `l_svg.h` | SVG rasterization — icons, diagrams, and vector assets from memory buffers. Subset renderer (paths, shapes, gradients); no text, images, clipping, or masks. ARGB output compatible with `l_gfx.h` |
```

### 2. Quick-Start Example Added (Line 159–199)

New **SVG Rasterization** section mirrors `l_img.h` pattern:
- File → buffer → load via `l_svg_load_mem`
- Accepts optional width/height args (or defaults to 512×512)
- Display with `l_blit` to canvas
- Cleanup with `l_svg_free_pixels`

Shows intended API usage clearly and concisely.

### 3. Function Reference Section Added (Line 936–989)

Comprehensive **Function Reference — `l_svg.h`** including:

**Supported elements:** `<svg>`, `<g>`, `<path>`, `<rect>`, `<circle>`, `<ellipse>`, `<line>`, `<polyline>`, `<polygon>`, `<defs>`, `<linearGradient>`, `<radialGradient>` (basic).

**Unsupported elements (intentional subset):** `<text>`, `<image>`, `<symbol>`, `<use>`, `<clipPath>`, `<mask>`, `<filter>`, CSS stylesheets, ICC profiles.

**Sizing behavior documented:**
- Both zero → intrinsic dimensions (or NULL)
- One zero → aspect-ratio preservation  
- Both nonzero → direct raster size (may distort)

**Function signatures:**
```c
uint32_t *l_svg_load_mem(data, len, opt, out_w, out_h)
void l_svg_free_pixels(pixels, w, h)
```

**Type documentation:**
```c
typedef struct {
    int width;        // 0 = use intrinsic
    int height;       // 0 = use intrinsic
    float dpi;        // default 96.0f
    const char *id;   // NULL = whole document
} L_SvgOptions;
```

**Usage notes cover:**
- ARGB pixel compatibility with `l_blit`
- 256 MB demand-paged allocator (same as `l_img.h`)
- DPI scaling for resolution multipliers
- Transform support (translate, scale, rotate, skew, matrix)
- CSS color names and RGB/RGBA hex
- Opacity, stroke, fill (SVG spec defaults)

### 4. gen-docs.ps1 Parser Extended

Added scanning for `l_svg.h`:
```powershell
$svgFunctions = Parse-Header -Path (Join-Path $root 'l_svg.h') -RequireWithDefs
```

Ready for future auto-generation if README SVG reference markers are added.

### 5. New Example: `examples/svg_view.c`

Minimal, self-documenting SVG viewer (68 lines):
- Command-line file input + optional width/height
- Subset limitations listed in help text
- Mirrors `examples/img_view.c` structure
- Error handling consistent with other examples
- Inline comments on NanoSVG subset for first-time users

## Subset Limitations Clearly Stated

User-facing documentation now explicitly lists:

✅ **Supported:**
- Basic shapes (path, rect, circle, ellipse, line, polyline, polygon)
- Groups (`<g>`) with transforms
- Gradients (linear and radial, basic)
- Transforms (all 6 types: translate, scale, rotate, skewX, skewY, matrix)
- CSS color names and hex RGB/RGBA
- Opacity and stroke/fill attributes

❌ **Unsupported (by design — NanoSVG subset):**
- Text rendering (`<text>`, `<tspan>`)
- Embedded raster images (`<image>`)
- Symbol/use/reference (`<symbol>`, `<use>`, `<image>` as reference)
- Clipping and masking (`<clipPath>`, `<mask>`)
- Complex filters and effects
- CSS stylesheets (inline attributes only)
- Advanced color spaces (ICC profiles)
- Font-dependent features

This is now discoverable via:
1. README header table (one line: "no text, images, clipping, or masks")
2. Function reference (two-column list of supported/unsupported)
3. Example help text (`svg_view` usage message)

## Quality Checks

- ✅ No build scripts modified (only gen-docs.ps1 for future use)
- ✅ No tests modified (subset limitations documented separately)
- ✅ Example follows existing patterns (img_view.c template)
- ✅ Consistent tone and formatting with rest of README
- ✅ API docs match actual `l_svg.h` header (width/height/dpi/id fields)
- ✅ Subset limitations transparent and front-and-center
- ✅ gen-docs.ps1 ready for future auto-generation

## Files Modified

| File | Changes |
|------|---------|
| `README.md` | Header table (+1 row), quick-start example (+41 lines), function reference (+56 lines) |
| `gen-docs.ps1` | Parser extension to scan `l_svg.h` (+6 lines) |
| `examples/svg_view.c` | **New file** (68 lines) |

## Ready for

- ✅ CI/build validation
- ✅ `gen-docs.ps1 -Check` validation
- ✅ User review and feedback
- ✅ Future backend improvements (PlutoSVG, resvg) via API-stable wrapper

---

**Implementation date:** 2026-07-26  
**Reviewed by:** Brett (Docs)  
**Next step:** Coordinate with Parker (Engine) for any API refinements or build integration.
