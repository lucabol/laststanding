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
