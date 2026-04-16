# SVG Docs: Pre-Landing Checklist

**For:** Parker, Dallas, Luca  
**From:** Brett (Docs)  
**Status:** Ready for implementation review  

## Documentation Surfaces Identified

Once `l_svg.h` lands, these must be updated:

| Surface | Update | Complexity |
|---------|--------|-----------|
| README header table (line 5) | Add `l_svg.h` row | Trivial |
| README quick-start (line 124+) | New SVG example after `l_img.h` section | Low |
| README function reference (line 893+) | New `## Function Reference — l_svg.h` section | Low (auto-gen) |
| gen-docs.ps1 | Extend parser to scan `l_svg.h` | Medium |
| examples/ | New `svg_view.c` (mirrors `img_view.c`) | Low |
| Compile flags section | Clarify float-math shim story | Trivial |

## Key Caveats (Must Document)

1. **Subset only:** No `image`, `text`, `symbol`, `use`, `clipPath`
2. **Sizing:** Zero dims = intrinsic size; mixed zero/nonzero = aspect ratio preservation; default DPI 96
3. **Float math:** Uses `sinf`/`cosf`/`powf`/etc. (ARM32 implication: math div ops)
4. **Memory:** 256 MB mmap pool (demand-paged), auto-reset between decodes
5. **Color:** ARGB output; CSS names supported; no ICC profiles

## Example Code Pattern

```c
L_SvgOptions opt = { .width = 512, .height = 512, .dpi = 96.0f, .id = NULL };
uint32_t *pixels = l_svg_load_mem(buf, buflen, &opt, &w, &h);
// ... use pixels ...
l_svg_free_pixels(pixels, w, h);
```

## Pre-Landing Decisions Needed

- **API signature:** Confirm struct + function names stable
- **Float shims:** Auto-define in `l_svg.h` or require user flag?
- **Versioning:** Mark as experimental or stable?
- **Vendoring:** Submodule, `compat/`, or inlined in header?

## Timeline

- **At land:** Brett applies README + gen-docs.ps1 updates + example
- **Post-land (48h):** Regenerate docs and commit

---

See `.squad/decisions/inbox/brett-nanosvg-docs.md` for full planning doc.
