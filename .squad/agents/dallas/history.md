# Project Context

- **Owner:** Luca Bolognese
- **Project:** laststanding — A freestanding C runtime. Minimal reimplementations of libc functions with direct syscall wrappers. No libc/glibc dependency. Statically linked, stripped, stdlib-free.
- **Stack:** C, inline assembly, cross-platform (Linux x86_64, ARM, AArch64, Windows)
- **Key file:** `l_os.h` — single header containing everything (string/memory functions, number conversion, syscall wrappers, file openers, platform startup code)
- **Build:** `./Taskfile test` (Linux), `build.bat` / `test_all.bat` (Windows)
- **Compiler flags:** `-Wall -Wextra -Wpedantic -ffreestanding -nostdlib -fno-builtin`
- **Conventions:** `l_` prefix for all functions, `L_FD` type for file descriptors, `L_MAINFILE` compile guard, UTF-8 internally
- **Created:** 2026-03-11

## Learnings

<!-- Append new learnings below. Each entry is something lasting about the project. -->
- `windows-latest` GHA runners ship with LLVM/clang pre-installed — no extra install step needed for the Windows build.
- `build.bat` compiles with `clang -I. -O3 -lkernel32 -ffreestanding` — no MSVC needed.
- `test_all.bat` calls `build.bat` internally, then runs every exe in `bin\`.
- `verify.bat` uses `dumpbin` (MSVC SDK) or `objdump` (MinGW) for dependency analysis; falls back gracefully if neither is available.
- Batch scripts need `shell: cmd` and `call` prefix in GitHub Actions to avoid early exit on the first command.
