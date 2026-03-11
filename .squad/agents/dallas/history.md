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
