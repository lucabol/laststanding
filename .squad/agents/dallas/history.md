# Project Context

- **Owner:** Luca Bolognese
- **Project:** laststanding — A freestanding C runtime. Minimal reimplementations of libc functions with direct syscall wrappers. No libc/glibc dependency. Statically linked, stripped, stdlib-free.
- **Stack:** C, inline assembly, cross-platform (Linux x86_64, ARM, AArch64, Windows)
- **Key file:** `l_os.h` — single header containing everything (string/memory functions, number conversion, syscall wrappers, file openers, platform startup code)
- **Build:** `./Taskfile test` (Linux), `build.bat` / `test_all.bat` (Windows)
- **Compiler flags:** `-Wall -Wextra -Wpedantic -ffreestanding -nostdlib -fno-builtin`
- **Conventions:** `l_` prefix for all functions, `L_FD` type for file descriptors, `L_MAINFILE` compile guard, UTF-8 internally
- **Created:** 2026-03-11

## Work Session — 2026-03-11T11:57:00Z

Rewrote `build.bat` for universal compatibility. Auto-detects compiler (clang/cl on PATH → vswhere → vcvarsall fallback). Replaced hard-coded test file list with dynamic loop. All scripts pass.

## Work Session — 2026-03-11T12:13:00Z

Created unified `build.ps1` PowerShell wrapper bridging Windows and Linux builds. Parameters: `-Target windows|linux|arm|all`, `-Action build|test|verify|all`, `-Compiler gcc|clang`, `-OptLevel 0-3`, `-WSLDist <name>`, `-Verbose`. Auto-translates paths (`C:\` → `/mnt/c/`), converts CRLF to LF before WSL invocation (critical for syscall macros). Updated README.md with full documentation and examples. All tests pass; ARM gracefully skips on Windows.

## Learnings

<!-- Append new learnings below. Each entry is something lasting about the project. -->
- `windows-latest` GHA runners ship with LLVM/clang pre-installed — no extra install step needed for the Windows build.
- `build.bat` compiles with `clang -I. -O3 -lkernel32 -ffreestanding` — no MSVC needed.
- `test_all.bat` calls `build.bat` internally, then runs every exe in `bin\`.
- `verify.bat` uses `dumpbin` (MSVC SDK) or `objdump` (MinGW) for dependency analysis; falls back gracefully if neither is available.
- Batch scripts need `shell: cmd` and `call` prefix in GitHub Actions to avoid early exit on the first command.
- `build.bat` now auto-detects compilers: PATH clang/cl → VS-bundled clang via vswhere → vcvarsall fallback. No special command prompt needed.
- VS 2022 Enterprise bundles clang at `VC\Tools\Llvm\x64\bin\clang.exe` — usable without vcvarsall.
- `build.bat` uses a `for %%f in (test\*.c)` loop instead of hard-coded file list — new test files are picked up automatically.
- **Decision pending:** Lambert raised l_strstr("", "") behavior deviation (returns NULL vs. standard's haystack pointer). Dallas to decide fix vs. document.
- `verify.bat` now auto-detects dumpbin via vswhere/vcvarsall (same pattern as build.bat). Works from plain cmd.exe.
- Never use Unicode characters (✓, ✗, etc.) in .bat scripts — cmd.exe default codepage can't render them. Use ASCII: PASS/FAIL/WARN/SKIP.
- `test_all.bat` was already ASCII-clean — no changes needed there.
- WSL builds on Windows-mounted filesystems (/mnt/c/) require CRLF→LF conversion first. Git checkout with `core.autocrlf=true` writes CRLF; gcc and bash choke on `\r` in inline asm string literals and shebangs. `build.ps1` runs `sed -i 's/\r$//'` before any WSL step.
- `verify.bat` has a latent exit-code bug: `findstr` inside the last `for` loop leaks errorlevel 1 when it finds no stdlib refs (the success case). The script never explicitly resets it. Needs `exit /b 0` at the end.
- Linux x86_64 syscall macros in `l_os.h` (lines 633+) are broken — missing GCC statement-expression wrappers `({ ... _ret; })`. `return my_syscall1(...)` expands to `return long _ret;` which is invalid C. ARM CI works because it uses separate inline build commands, not the Taskfile. This needs fixing before Linux WSL builds can succeed.
- `build.ps1` created as unified PowerShell wrapper: `-Target windows|linux|arm|all`, `-Action build|test|verify|all`, `-Compiler gcc|clang`, `-OptLevel 0-3`. Uses `cmd /c call` for batch files, `wsl bash -c` for Linux/ARM via Taskfile.

## Work Session — 2026-03-12

Fixed two build blockers:

**build.ps1 CRLF fix (Issue 1):** The WSL-based `sed -i 's/\r$//'` approach for CRLF stripping was unreliable — quoting through PowerShell → WSL → bash → sed is fragile, and `sed -i` on `/mnt/c/` NTFS filesystems can fail silently. Replaced with pure PowerShell byte-level CR removal using `ReadAllBytes`/`WriteAllBytes`. This avoids all quoting and filesystem issues.

**l_os.h code fixes:** The actual compilation errors were NOT from CRLF — they were pre-existing code bugs:
- x86_64 syscall macros 0–5 were missing GCC statement-expression wrappers `({ ... _ret; })`. Only `my_syscall6` had them. Added wrappers to all six.
- AArch64 startup asm had two lines merged on one line (line 149) and a stray `"` before `);` in the asm closing.
- Removed dead duplicate `#elif defined(__arm__)` startup block (identical condition already handled earlier in the same `#if` chain).
- AArch64 syscall macros 5 and 6 were also missing `({` / `})` wrappers — fixed for consistency.

**verify.bat exit code (Issue 2):** `findstr` inside the last `for` loop set `errorlevel 1` when it found no stdlib refs (the success case). Added `exit /b 0` at the end of the script.

All targets verified: Windows verify PASS, Linux build PASS, Linux test PASS.

## Work Session — 2026-03-12 (follow-up)

Re-investigated reported Linux build breakage. Found that the previous fixes (statement-expression wrappers, AArch64 asm, verify.bat exit code) are ALL correctly in place in `l_os.h` and `verify.bat`. The file has LF line endings (no CRLF contamination). Verified by:
- Compiling `test/test.c` under WSL with full strict flags — exits 0, only `-Wpedantic` warnings about GNU statement expressions (expected, harmless).
- Running the compiled test binary — all tests pass.
- Running `build.ps1 -Target windows -Action verify` — PASS, exit code 0.

The reported errors were likely from a stale build or a cached/uncommitted version of `l_os.h`. No code changes needed this session — everything was already fixed.

## Work Session — 2026-03-13

Fixed ARM (32-bit armhf) build target — was completely broken because `l_os.h` had no `#elif defined(__arm__)` branch in the syscall macros section (lines 591–890). Only x86_64 and aarch64 were handled; ARM fell through to `#error`.

**Changes:**

1. **l_os.h — added ARM 32-bit syscall macros:** Full `my_syscall0`–`my_syscall6` for ARM EABI (r0–r5 for args, r7 for syscall number, `svc #0`). Used `push {r7}` / `pop {r7}` pattern to save/restore r7 inside the asm block, avoiding the well-known Thumb mode frame pointer conflict (`r7 cannot be used in 'asm'`). Also added ARM O_* file flag definitions (same values as x86_64, O_DIRECTORY=0x4000).

2. **Taskfile — added `-fomit-frame-pointer` to `build_arm`:** Belt-and-suspenders alongside the push/pop pattern.

**Verified:** All 3 test files compile to `ELF 32-bit LSB executable, ARM, EABI5, statically linked, stripped`. Linux x86_64 build+test still passes (no regressions). QEMU not installed in WSL so runtime tests skipped.

## Learnings

- ARM 32-bit syscall macros must use `push {r7}` / `mov r7, <nr>` / `svc #0` / `pop {r7}` pattern (not `register ... asm("r7")`) because Thumb mode uses r7 as frame pointer. GCC rejects r7 in register constraints AND clobber lists when it's the frame pointer.
- ARM file flags (O_RDONLY, O_WRONLY, O_CREAT, etc.) use the same values as x86_64. O_DIRECTORY is 0x4000 on ARM vs 0x10000 on aarch64.
- `build.ps1` was already correctly wired — the ARM build failure was purely a missing code section in `l_os.h`, not a build script problem.

## Work Session — 2026-03-14

Fixed three bugs: x86_64 segfault, ARM memcmp, and test failure detection.

**Bug 1 — x86_64 segfault at -O3:** Root cause was a stack misalignment bug in the `_start` assembly. The `sub $8, %rsp` instruction after `and $-16, %rsp` caused the stack to be misaligned by 8 bytes when entering `main`. The x86-64 ABI requires that at the CALL instruction, %rsp is 16-byte aligned; after CALL pushes the 8-byte return address, %rsp % 16 == 8 at function entry. The extra `sub $8` broke this contract. At -O3, GCC auto-vectorizes loops (e.g., memset/strlen) using aligned SSE/AVX stores (`movaps`), which segfault on misaligned addresses. Fix: removed the `sub $8, %rsp` line.

**Bug 2 — ARM l_memcmp wrong result:** `l_memcmp` used `char c1` to store the subtraction result. On ARM where `char` is unsigned by default, negative results (e.g., 'c'-'d' = -1) were stored as 255, returning positive instead of negative. Fix: changed `c1` to `int` and casts to `unsigned char *` per the C standard's memcmp specification.

**Bug 3 — Taskfile/build.ps1 failure detection:** The Taskfile's `test` and `test_arm` loops didn't track exit codes — failures were swallowed by the loop continuing. Also changed test globs from `bin/*` to `bin/test*` so non-test utilities (countlines) aren't run. Added failure flag propagation with `exit 1`. In `build.ps1`, added output scanning for failure indicators ("Segmentation fault", "FAILED", "core dumped") as a safety net in `Invoke-Step`, independent of exit codes.

**Also fixed:** `l_strstr` had an out-of-bounds read when needle was longer than remaining haystack — l_memcmp would read past the buffer. Added haystack length tracking to stop searching when fewer chars remain than needle length. Preserves the documented `l_strstr("", "") == NULL` behavior (pending decision #12).

Verified: Linux x86_64 test PASS (all tests including extended at -O3), ARM test_arm PASS (memcmp passes), Windows test PASS, build.ps1 correctly reports PASS/FAIL.

## Learnings

- x86_64 `_start` must NOT have `sub $8, %rsp` before `call main`. The ABI requires %rsp to be 16-byte aligned at the point of `call`; the `call` instruction itself pushes 8 bytes, giving the callee rsp%16==8 as expected. Adding `sub $8` causes rsp%16==0 at entry — wrong alignment that triggers segfaults from vectorized SIMD stores at -O3.
- `l_memcmp` must use `int` (not `char`) for the difference result and `unsigned char *` casts. On ARM, `char` is unsigned by default, so storing a negative subtraction result in `char` wraps to a large positive value. The C standard mandates unsigned char comparison for memcmp.
- `l_strstr` must bound its memcmp calls to not read past the haystack. When needle is longer than remaining haystack, the memcmp would read out-of-bounds memory. Track haystack length and stop when fewer chars remain than needle length.
- Taskfile test loops must track exit codes with a failure flag and `exit 1` — bash `for` loops don't propagate non-zero exit codes from loop body commands.
- The Taskfile `test` glob should be `bin/test*` not `bin/*` to avoid running non-test utilities that require arguments.
- Never use Unicode characters (✓, ✗, etc.) in C test output either — when WSL output flows through PowerShell, encoding gets mangled (same codepage issue as .bat files). Use ASCII: `[OK]` / `[FAIL]`.
- `build.ps1` ARM verify must call `./Taskfile verify_arm` (not `verify`), which checks `bin/*.armhf` files for ARM ELF type, static linking, stripped, no stdlib symbols. The `verify` function only checks x86_64 binaries.
- The ARM 'all' action list in build.ps1 must include 'verify' alongside 'build' and 'test' — it was originally missing.
- PowerShell Core (pwsh) provides `$IsLinux`, `$IsWindows`, `$IsMacOS` as automatic variables. Windows PowerShell 5.1 does NOT define them — use `Test-Path variable:IsLinux` to detect availability and fall back to `$true` for Windows.
- On GitHub Actions, all runners (ubuntu, windows, macos) have `pwsh` pre-installed — `shell: pwsh` works everywhere without an install step.
- When ci.ps1 runs natively on Linux (not via WSL), it must call `bash -c "cd '$RepoRoot' && ./Taskfile ..."` directly — no WSL prefix, no CRLF stripping, no WSL path translation.

## Work Session — 2026-03-15

Enhanced `build.ps1` concise (non-ShowAll) output to show inline stats per step instead of bare PASS/FAIL.

**Changes to `Invoke-Step` concise branch:**
- Build steps show compiler command + file count: `clang -I. -O3 -ffreestanding ...  3 files  PASS`
- Test steps show binary count + assertion count: `3 binaries, 176 assertions  PASS`
- Verify steps show binary count + max size + link type: `3 binaries, max 22KB, KERNEL32 only  PASS`
- Labels padded to 22 chars for alignment

**New helper functions:** `Get-BuildSummary`, `Get-TestSummary`, `Get-VerifySummary`, `Get-StepSummary`, `Collect-BinarySizes`, `Write-BinarySizeTable`

**Binary size comparison table (new):** Printed before the summary when ≥2 targets were verified. Right-aligned comma-separated byte sizes, one column per target.

**Unchanged:** `-ShowAll` mode, failure detection, full-output-on-failure behavior. ShowAll mode also collects binary sizes for the comparison table.

## Learnings

- Windows `verify.bat` outputs file sizes as `File: test.exe  16384 bytes`; Linux/ARM Taskfile `verify` outputs `Binary size: 13401 bytes (text: ...)`. Both patterns must be handled.
- `build.bat` echoes `Using compiler: <CC>` and then indented filenames; Taskfile `build` produces no output on success — file count falls back to `Get-ChildItem test\*.c`.
- Test output uses `[OK]` per assertion and `--- Running <exe> ---` per binary — regex counts of these give assertion and binary counts.

## Work Session — 2026-03-15 (follow-up)

Two changes: multi-compiler matrix and build.ps1 → ci.ps1 rename.

**1. Multi-compiler builds:** ci.ps1 now runs BOTH gcc AND clang for Linux and ARM targets by default (`-Compiler all`). The Taskfile's `build_arm` handles clang cross-compilation with `--target=arm-linux-gnueabihf --sysroot=/usr/arm-linux-gnueabihf -flto`. The `verify` and `verify_arm` functions now pass through `"$@"` so the compiler parameter propagates to the rebuild step. Summary table shows `Linux (gcc)`, `Linux (clang)`, `ARM (gcc)`, `ARM (clang)` as separate targets. Binary size table uses short column names: `Win/clang`, `Lin/gcc`, `Lin/clang`, `ARM/gcc`, `ARM/clang`.

**2. Renamed build.ps1 → ci.ps1:** The script does build + test + verify across all platforms — "ci" is more accurate. `build.ps1` kept as a one-line backward-compat wrapper (`& "$PSScriptRoot\ci.ps1" @args`). README.md updated.

Verified: `.\ci.ps1 -Target linux` runs both gcc and clang, all 6 steps PASS. Backward-compat wrapper `.\build.ps1` also works.

## Learnings

- ARM clang cross-compilation uses `clang --target=arm-linux-gnueabihf --sysroot=/usr/arm-linux-gnueabihf -flto` — no `-lgcc` needed. The sysroot comes from the `gcc-arm-linux-gnueabihf` package.
- When parameterizing Taskfile functions for clang vs gcc, pass the explicit compiler name (`arm-linux-gnueabihf-gcc`) not an empty string — bash word splitting collapses empty args, shifting positional parameters.
- `ci.ps1` is the canonical name for the unified CI script; `build.ps1` is a backward-compat wrapper.
- clang produces noticeably smaller freestanding binaries than gcc (e.g., 15KB vs 22KB for test_extended at -O3).

## Work Session — 2026-03-16

Fixed ARM clang cross-compilation failures caused by GCC-specific inline assembly syntax in `l_os.h`.

**Changes:**

1. **l_os.h — ARM `_start` assembly syntax (lines 131-139):** Replaced GCC-specific register/immediate prefixes with universal syntax that both gcc and clang accept:
   - `%r0` → `r0`, `%r1` → `r1`, `%r2` → `r2`, `%r3` → `r3`, `%sp` → `sp` (bare register names)
   - `$4` → `#4`, `$-8` → `#-8`, `$1` → `#1`, `$0x00` → `#0` (`#` for ARM immediates)
   The Thumb block (lines 126-128) was already using correct bare register syntax.

2. **l_os.h — Removed `.global _start` after `.weak _start` (ARM + AArch64):** Clang's assembler rejects changing symbol binding from weak to global. `.weak` already implies global visibility, so `.global` was redundant.

3. **test/test.c — 5 functions, test/test_extended.c — 15 functions:** Added `(void)` to all empty parameter lists (`void test_foo()` → `void test_foo(void)`). Clang warns about function declarations without prototypes (`-Wstrict-prototypes`).

**Syscall macros (my_syscall0–6) were already correct** — they used bare register names and `#0` syntax, which work with both compilers.

**Verified:** All 3 test files compile clean with both ARM clang and ARM gcc. x86_64 gcc build+test passes (no regressions).

## Learnings

- ARM inline asm must use bare register names (`r0`, `sp`) not `%r0`/`%sp`, and `#` for immediates not `$`. GCC accepts both syntaxes; clang only accepts the ARM-native syntax. The `%` and `$` prefixes are x86 conventions that GCC's ARM backend also accepts but clang does not.
- Clang's assembler rejects `.weak sym` followed by `.global sym` — it treats this as a binding change error. `.weak` already implies global visibility, so `.global` is redundant and should be omitted.
- C functions with empty parameter lists `void f()` should use `void f(void)` for strict C correctness. Clang warns about this with `-Wstrict-prototypes`; GCC is silent by default.

## Work Session — 2026-03-17

Unified CI: made ci.ps1 cross-platform and consolidated GitHub Actions workflows.

**ci.ps1 changes:**
- Added OS detection using `$IsLinux`/`$IsWindows` (PowerShell Core automatic vars), with fallback for Windows PowerShell 5.1 (which lacks them).
- When `$IsLinux`: Linux/ARM functions call `./Taskfile` directly (no WSL wrapper, no CRLF stripping). Windows target is skipped. ARM compiler and QEMU checks run natively via `bash -c`.
- When `$IsWindows`: all existing WSL-based behavior preserved unchanged.
- WSL path translation (`Get-WslPath`) and WSL availability check only run on Windows.

**Workflow consolidation:**
- Created single `.github/workflows/ci.yml` replacing `windows-ci.yml`, `linux-ci.yml`, `arm-ci.yml`.
- Three jobs: `windows` (windows-latest, pwsh), `linux` (ubuntu-latest, pwsh), `arm` (ubuntu-latest + cross-compilers, pwsh).
- All jobs use `./ci.ps1 -Target <target>` as the single entry point.

**Verified:** Windows build+test+verify all PASS. Linux build via WSL PASS (confirming Windows code path unchanged).

## Work Session — 2026-03-16 (follow-up)

Added two new `l_*` functions to `l_os.h` with full test coverage.

**Changes:**

1. **l_strcmp(const char *s1, const char *s2):** Full string comparison using unsigned char, returns <0/0/>0. Declaration, override `#define`, and implementation placed next to `l_strncmp`.

2. **l_strncpy(char *dst, const char *src, size_t n):** Copies at most n chars from src to dst, zero-pads remainder per C standard. Declaration, override `#define`, and implementation placed next to `l_strcpy`.

3. **l_getenv — deferred.** Linux `_start` doesn't capture `envp`; `/proc/self/environ` approach requires static buffers and is Linux-only. Windows needs `GetEnvironmentVariableW` + static buffer. Both are cross-cutting changes better handled as standalone work. Decision documented in `.squad/decisions.md`.

4. **Tests:** Added `test_strcmp` (12 assertions) and `test_strncpy` (7 assertions) to `test/test_extended.c` following existing patterns.

Verified: Windows build + all tests pass (build.bat && test_all.bat).

## Work Session — 2026-03-17 (showcase programs)

Created two new showcase programs: `test/base64.c` (RFC 4648 base64 encoder/decoder) and `test/sort.c` (line-based sort with shell sort).

**test/base64.c:** Streams via buffered I/O (4KB read/write buffers). Encodes 3 bytes → 4 base64 chars with 76-char line wrapping. Decodes skipping whitespace, handles `=` padding. Round-trip verified on l_os.h (binary-safe, `fc /b` confirmed no differences).

**test/sort.c:** Reads up to 64KB into static buffer, splits into lines (max 4096), sorts with shell sort (Knuth's gap sequence). Supports `-r` (reverse), `-f` (fold case via `l_strcasecmp`), `-n` (numeric via `l_atoi`), `-u` (unique — uses sort comparison for dedup).

Both follow the `test/ls.c` pattern: `#define L_MAINFILE`, `--help` support, `l_open_read` for file input, `L_STDIN` for stdin.

Verified: Windows clang build clean (no warnings), all core tests pass, base64 round-trip confirmed, sort tested with all flag combinations.

## Learnings

- Shell sort with Knuth's gap sequence (`gap = gap * 3 + 1`, shrink by `/3`) is a good default for freestanding tools — O(n^1.25) average, no worst case on sorted input, and `/3` is a constant divisor that compilers optimize to multiply-shift (ARM-safe).
- Buffered I/O pattern (static 4KB read/write buffers with `ibuf_get`/`obuf_put` helpers) is clean for streaming tools like base64. Avoids per-byte syscalls without reading entire input into memory.
- Line splitting with `l_strchr(p, '\n')` naturally treats `\n` as a terminator: after the last `\n`, the pointer advances to the buffer's null terminator and the loop exits. No special trailing-newline logic needed.
- `test_all.bat` runs all `bin\*.exe` — hangs on stdin-reading tools (base64, sort, grep, hexdump, wc, upper, countlines). The Taskfile's `bin/test*` glob avoids this. This is a pre-existing issue.

## Work Session — 2026-03-24T17:04:00Z

Completed showcase program delivery: committed base64 encoder/decoder and sort utility. Fixed test_all.bat to use `test*.exe` pattern matching (prevents hang on stdin-reading tools). 15/15 tests pass. Commit: 15bdf74.
