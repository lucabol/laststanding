# Parker History

## Seed Context

- Requested by: Luca Bolognese
- Project: laststanding
- Tech stack: Freestanding C runtime/library, static builds, Windows/Linux/ARM/AArch64/RISC-V
- Current focus: NanoSVG-backed SVG support

## Learnings

- 2026-04-16: Squad bootstrapped from repository context because the user was unavailable to confirm the roster in-session.
- 2026-04-16: `l_img.h` is a raster-only, memory-buffer-in → owned-ARGB-out path with an internal mmap bump allocator; a NanoSVG path needs a separate `l_svg.h`, mutable NUL-terminated staging input, float-math shims, and Dallas-owned build wiring for `-Icompat`.
- 2026-04-16: The NanoSVG fork compiles freestanding once its hosted includes are stripped, allocator calls are macro-hooked, `sscanf` color parsing is replaced by local scanners, and `compat/math.h` supplies both float wrappers and `isnan`.
- 2026-04-16: The real `l_svg.h` path now stage-copies input, parses with NanoSVG, rasterizes through `nanosvgrast`, and returns owned ARGB pixels; explicit width+height keep exact output dimensions while NanoSVG itself still uses uniform scaling inside that box.
- 2026-04-16: The shipped NanoSVG fork is now memory-only end to end: example/docs no longer reference file parsing, and focused validation passes with real parse/raster behavior for solid fills, viewBox sizing, aspect-ratio inference, and linear gradients.
- 2026-04-16: To keep the fork freestanding-clean, `compat/nanosvg/*.h` now require `NSVG_MALLOC/REALLOC/FREE` hooks from `l_svg.h` instead of falling back to hosted `malloc`/`free`, and the old debug file-dump code in `nanosvgrast.h` is gone.
