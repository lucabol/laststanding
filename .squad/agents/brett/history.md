# Brett History

## Seed Context

- Requested by: Luca Bolognese
- Project: laststanding
- Tech stack: Freestanding C runtime/library, static builds, Windows/Linux/ARM/AArch64/RISC-V
- Current focus: NanoSVG-backed SVG support

## Learnings

- 2026-04-16: Squad bootstrapped from repository context because the user was unavailable to confirm the roster in-session.
- 2026-07-26: NanoSVG integration planning — identified six major documentation surfaces requiring updates once l_svg.h lands. Created decision inbox document with API shape, caveats, and timeline. Key surface updates: README header table row, quick-start SVG example (mirroring img_view.c), new Function Reference section, and gen-docs.ps1 parser extension. Documented subset limitations (no image/text/clipPath/symbol/use), sizing policy, and float-math implications for ARM32.
- 2026-07-26 (implementation): Landed SVG documentation across README, gen-docs.ps1, and new svg_view.c example. README now shows six headers (was five); SVG quick-start example mirrors img_view.c pattern. Subset limitations clearly documented: supported elements (g, path, rect, circle, ellipse, line, polyline, polygon, defs, basic gradients), unsupported (text, image, symbol, use, clipPath, mask, filter, CSS, ICC profiles). L_SvgOptions API documented with sizing behavior (0=intrinsic, mixed=aspect-preserve, both=scale). Usage notes cover ARGB output, 256MB demand-paged allocator, DPI scaling, transform support, CSS colors. Example code shows file→load→display pattern consistent with l_img.h ecosystem.
- 2026-07-26 (cleanup pass): Validated gen-docs.ps1 pipeline for SVG auto-generation. Added conditional SVG parsing (graceful no-op if l_svg.h not yet present), built SVG markdown table builder, added Update-ReadmeSection call for SVG reference, wrapped SVG function reference in markers (<!-- BEGIN/END SVG REFERENCE -->) so gen-docs can auto-regenerate once header lands. Script now reports SVG function count when present. Verified gen-docs runs successfully without l_svg.h — markers preserve manually-written docs pending auto-generation on header arrival.
- 2026-07-26 (final validation): Confirmed l_svg.h now present in codebase with actual NanoSVG implementation. Fixed gen-docs parsing for l_svg.h: removed -RequireWithDefs constraint since doc-comments live in public API section (outside L_WITHDEFS), matching l_img.h pattern. Script now correctly emits "2 functions from l_svg.h" and auto-generates SVG reference table. Removed non-existent `id` field from L_SvgOptions docs to match actual struct (width/height/dpi only). Updated both README and svg_view.c example to use correct struct. Verified svg_view binary compiles (53KB Linux binary) on all platforms.
