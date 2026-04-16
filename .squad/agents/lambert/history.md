# Lambert History

## Seed Context

- Requested by: Luca Bolognese
- Project: laststanding
- Tech stack: Freestanding C runtime/library, static builds, Windows/Linux/ARM/AArch64/RISC-V
- Current focus: NanoSVG-backed SVG support

## Learnings

- 2026-04-16: Squad bootstrapped from repository context because the user was unavailable to confirm the roster in-session.
- 2026-04-16: Drafted SVG validation plan for NanoSVG backend. Key decisions: (1) Keep `l_svg.h` separate from `l_img.h`; (2) Subset coverage explicit (no text, image, clipPath, symbol, use, CSS); (3) Math shims required for all float functions (`sinf`, `cosf`, `sqrtf`, `ceilf`, `floorf`, `fmodf`, `powf`, `atan2f`) over `l_os.h`; (4) Graceful error handling for unsupported elements (log or ignore, never crash); (5) Cross-platform test gate on all 7 targets (Windows MSVC, Linux x86_64 GCC/Clang, ARM32 GCC/Clang, AArch64 GCC/Clang) with zero warnings and no stdlib references in binary. Plan includes 60+ test cases spanning sizing, elements, invalid data, colors, transforms, memory safety, and integration with `l_img.h`.
- 2026-04-16: NanoSVG on ARM32 clang needed freestanding `__aeabi_f2lz`/`__aeabi_f2ulz` helpers in `l_os.h`; after adding them, direct `test_svg` binaries passed on Windows, Linux, ARM32, AArch64, RISC-V, and ARM32 clang cross-build/QEMU, while full CI still stayed red for unrelated TLS/FS/build issues.
- 2026-04-16: tests\\test_svg.c should stay on the three-field L_SvgOptions API (width, height, dpi) and use TEST_FUNCTION only as an in-function logging macro, not as a function declaration helper.
- 2026-04-16: Replaced SVG test placeholders with inline SVG fixtures that assert real decode failures, viewBox sizing, inferred aspect ratio, and pixel colors from NanoSVG output.
- 2026-04-16: Real l_svg.h coverage should only treat binary garbage, NULL, and empty buffers as guaranteed invalid inputs; NanoSVG may still accept malformed or unsized XML, so those cases should not be asserted as NULL failures.
