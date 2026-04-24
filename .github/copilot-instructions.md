# Copilot Instructions for laststanding

## What This Is

A freestanding C runtime — minimal reimplementations of libc functions with direct syscall wrappers. No libc/glibc dependency. Binaries are statically linked, stripped, and stdlib-free. Targets Linux (x86_64, ARM, AArch64) and Windows.

## Workflow

**Always follow this sequence for any code change:**
1. Run `powershell -NoProfile -ExecutionPolicy Bypass -File ci.ps1` (full cross-platform CI)
2. If CI passes, regenerate `docs/` via `powershell -NoProfile -ExecutionPolicy Bypass -File gen-docs.ps1` (updates `docs/API.md`, `docs/COMPAT.md`, `docs/COVERAGE.md`)
3. Commit and push

Never commit without running CI first. Never skip ARM/AArch64 targets — they catch real bugs.

**Treat compiler warnings as errors.** Code must compile cleanly with `-Wall -Wextra -Wpedantic` on all 7 targets. If CI shows warnings, fix them before committing.

## Build & Test

### Full CI (recommended)

```sh
powershell -NoProfile -ExecutionPolicy Bypass -File ci.ps1           # all 7 targets
powershell -NoProfile -ExecutionPolicy Bypass -File ci.ps1 -Target windows  # Windows only
powershell -NoProfile -ExecutionPolicy Bypass -File ci.ps1 -Target linux    # Linux gcc+clang
powershell -NoProfile -ExecutionPolicy Bypass -File ci.ps1 -Target arm      # ARM+AArch64 gcc+clang
```

### Individual platforms

```bat
cmd /c "call test_all.bat"   # Windows: build + test + smoke
cmd /c "call verify.bat"     # Windows: check stdlib independence
```

```sh
./Taskfile test              # Linux: build + test (gcc, -Oz by default)
./Taskfile test_arm          # ARM32: build + test via QEMU
./Taskfile test_aarch64      # AArch64: build + test via QEMU
```

There is no single-test runner. Each `.c` file in `test/` compiles to one binary in `bin/`. Run a single test by building and then executing `bin/<name>` directly.

## Architecture

Everything lives in one header: `l_os.h`. It provides:

- **String/memory functions** (`l_strlen`, `l_memcpy`, `l_strchr`, etc.)
- **Number conversion** (`l_atoi`, `l_itoa`, etc.)
- **Syscall wrappers** (`l_open`, `l_read`, `l_write`, `l_exit`, etc.)
- **Convenience file openers** (`l_open_read`, `l_open_write`, `l_open_append`, etc.)
- **Platform startup code** (inline asm `_start` for Linux archs, `mainCRTStartup` for Windows)

### Compile-time guards

- `#define L_MAINFILE` before `#include "l_os.h"` — activates both `L_WITHDEFS` (function declarations) and `L_WITHSTART` (entry point / startup code). Exactly one translation unit must define this.
- `#define L_DONTOVERRIDE` — prevents the header from `#define`-ing standard names (`strlen`, `memcpy`, `puts`, `exit`, etc.) to their `l_` equivalents.

Without `L_MAINFILE`, including `l_os.h` is safe and idempotent (it only brings in freestanding standard headers and type definitions).

### Syscall layers

Each architecture has its own set of `my_syscall0`–`my_syscall6` macros (inline asm). The public `l_*` functions call these macros. On Windows, the functions call Win32 API directly (`CreateFileW`, `ReadFile`, `WriteFile`, etc.) with UTF-8 ↔ UTF-16 conversion at the boundary.

## Conventions

- All library functions use the `l_` prefix. Unless `L_DONTOVERRIDE` is defined, macros alias standard names to `l_` versions.
- File descriptor type is `L_FD` (aliased to `ptrdiff_t`), not `int` — this is needed for Windows `HANDLE` compatibility.
- Compiler flags are strict: `-Wall -Wextra -Wpedantic -ffreestanding -nostdlib -fno-builtin`. New code must compile cleanly under these.
- Tests use `TEST_ASSERT(condition, "description")` and `TEST_FUNCTION("name")` macros defined in `test/test.c`. Tests call `exit(-1)` on first failure.
- Test programs write `main()` as usual — the startup code in `l_os.h` calls it.
- The `misc/` directory contains reference implementations using standard libc for comparison.
- Strings are handled as UTF-8 internally, including on Windows (converted from UTF-16 at the entry point).

### Header ordering in l_os.h

Functions in `l_os.h` must be ordered after any `l_*` functions they call — the `L_OSH` block compiles top-to-bottom on first include. Adding new functions that call existing ones requires placing them below their dependencies. All function declarations AND definitions must use `static inline`.

### ARM32 constraints

- ARM32 has no hardware integer divide. `l_os.h` provides `__aeabi_uidiv`/`__aeabi_idiv` (32-bit) and `__aeabi_ldivmod`/`__aeabi_uldivmod` (64-bit) plus shift helpers (`__aeabi_llsl`/`__aeabi_llsr`/`__aeabi_lasr`).
- GCC defaults to **Thumb mode** on ARM32. Any naked asm functions using conditional instructions (`movmi`, `movpl`) must have `__attribute__((target("arm")))`.
- All `__aeabi_*` helpers need `__attribute__((used))` to survive LTO + gc-sections.
- ARM/AArch64 tests run under QEMU — requires `qemu-arm` and `qemu-aarch64` to be installed (via WSL on Windows).
