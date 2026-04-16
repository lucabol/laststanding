# Ripley History

## Seed Context

- Requested by: Luca Bolognese
- Project: laststanding
- Tech stack: Freestanding C runtime/library, static builds, Windows/Linux/ARM/AArch64/RISC-V
- Current focus: NanoSVG-backed SVG support

## Learnings

- 2026-04-16: Squad bootstrapped from repository context because the user was unavailable to confirm the roster in-session.
- 2025-07-17: Designed NanoSVG integration as `l_svg.h` — separate header from `l_img.h` because SVG needs caller-supplied dimensions. Key technical finding: `l_os.h` has all double-precision math but no float-suffixed versions (`sinf`, `cosf`, etc.) — need `compat/math.h` shim. NanoSVG `sscanf` usage is only 3 call sites in color parsing — fork-and-replace is cleaner than implementing sscanf. Full design in `.squad/decisions/inbox/ripley-nanosvg-design.md`.
- 2025-07-17: Finalized implementation-ready handoff: 6 new files, 4 existing files to modify, 13-step execution order with hard gates. NanoSVG fork has 13 parser changes and 6 rasterizer changes. Build detection is `l_img\.h || l_svg\.h` → `-Icompat` (Taskfile lines 79/128/174/221, build_parallel.ps1 line 37). Test runner lists at Taskfile lines 260/547/567/593/619 and test_all.bat line 24.
- 2025-07-17: Audited worktree state — implementation is scaffolding, not shippable. NanoSVG fork is unmodified upstream (still has malloc/free/sscanf/stdio.h/nsvgParseFromFile). l_svg.h has placeholder implementation (fills red pixels, never calls NanoSVG). Tests pass against the placeholder, not real SVG parsing. 8 concrete blockers identified. See audit findings in decisions inbox.
- 2025-07-17: Implemented real NanoSVG integration. Rewrote l_svg.h with full NanoSVG parse→rasterize→ARGB pipeline. Discovered the fork was actually much further along than audit indicated (allocator macros converted, sscanf replaced, system includes removed, nsvgParseFromFile removed). Only l_svg.h itself needed rewriting. CI passes all targets: Windows, Linux gcc/clang, ARM gcc/clang, RISC-V gcc/clang. 19 SVG tests pass. Docs regenerated.
- 2025-07-17: Review pass — approved with zero high-severity blockers. Implementation uses real NanoSVG (nsvgParse + nsvgRasterize), not custom parser. API matches locked design. Fork fully adapted. All pre-existing CI failures unrelated to SVG changes.
