# Copilot Instructions for laststanding

## What This Is

A freestanding C runtime — minimal reimplementations of libc functions with direct syscall wrappers. No libc/glibc dependency. Binaries are statically linked, stripped, and stdlib-free. Targets Linux (x86_64, ARM, AArch64) and Windows.

## Build & Test

### Linux/macOS

```sh
./Taskfile test              # build and run all tests (gcc, -O3 by default)
./Taskfile build clang 2     # build with clang at -O2
./Taskfile build_arm         # cross-compile for ARM
./Taskfile test_arm           # build + run ARM tests via QEMU
./Taskfile verify            # check binaries have no stdlib deps, are stripped, etc.
```

### Windows

```bat
REM Requires x64 Native Tools Command Prompt or clang on PATH
build.bat                    # build test binaries
test_all.bat                 # build + run all tests
verify.bat                   # check binaries for stdlib independence
```

There is no single-test runner. Each `.c` file in `test/` compiles to one binary in `bin/`. Run a single test by building and then executing `bin\<name>` directly.

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
