# laststanding

A minimal C runtime and test suite for exploring freestanding, static, and cross-platform C code. This project provides:

- Minimal implementations of common C library functions (with `l_` prefix) in `l_os.h`
- Direct syscall wrappers for Linux (x86_64, ARM/32-bit, AArch64/64-bit ARM) and Windows
- Test programs for verifying correctness and portability
- Build scripts for Linux and Windows

## Features
- No dependency on libc or glibc: all binaries are statically linked and freestanding
- Cross-platform syscall support: x86_64, ARM, AArch64, and Windows
- Simple build and test automation via `Taskfile` (bash) and `test_all.bat` (Windows)
- AArch64 (64-bit ARM) fully supported and tested via QEMU

## How to Use

### Quick Start

`laststanding` is a single-header library. Copy `l_os.h` into your project and include it:

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    puts("Hello from laststanding!\n");
    return 0;
}
```

### Compile-Time Flags

| Define | Purpose |
|--------|---------|
| `L_MAINFILE` | Activates both function definitions and platform startup code. **Exactly one** translation unit must define this before including `l_os.h`. |
| `L_DONTOVERRIDE` | Prevents `#define strlen l_strlen` style aliases. Use this if you need to mix `laststanding` with standard library headers. |

In multi-file projects, only one `.c` file defines `L_MAINFILE`. Other files include `l_os.h` without it — they get type definitions and constants but no function bodies or startup code:

```c
// utils.c — no L_MAINFILE, safe to include alongside other headers
#include "l_os.h"

// main.c — the one translation unit with startup code and definitions
#define L_MAINFILE
#include "l_os.h"
```

### Compiler Flags

Binaries must be compiled freestanding with no standard library:

**Linux (gcc or clang):**
```sh
gcc -I. -O3 -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o myapp myapp.c
```

**Windows (clang):**
```bat
clang -I. -O3 -lkernel32 -ffreestanding -Wall -Wextra -Wpedantic -o myapp.exe myapp.c
```

**ARM cross-compilation (gcc):**
```sh
arm-linux-gnueabihf-gcc -I. -O3 -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o myapp myapp.c
```

### Key Types

| Type | Purpose |
|------|---------|
| `L_FD` | File descriptor (`ptrdiff_t`). Used instead of `int` for Windows `HANDLE` compatibility. |
| `L_STDIN`, `L_STDOUT`, `L_STDERR` | Standard file descriptor constants (0, 1, 2). |

### Standard Name Aliases

By default, `l_os.h` defines macros that alias standard names to their `l_` equivalents (e.g., `strlen` → `l_strlen`, `exit` → `l_exit`). This lets you write familiar C code without the `l_` prefix. Define `L_DONTOVERRIDE` before including to disable this.

## Function Reference

Generated from `l_os.h` doc-comments. Run `.\gen-docs.ps1` to update.

<!-- BEGIN FUNCTION REFERENCE -->

| Function | Description | Platform |
|----------|-------------|----------|
| **String functions** | | |
| `l_wcslen` | Returns the length of a wide character string | All |
| `l_strlen` | Returns the length of a null-terminated string | All |
| `l_strcpy` | Copies src string to dst, returns dst | All |
| `l_strncpy` | Copies up to n characters from src to dst, padding with nulls | All |
| `l_strchr` | Returns pointer to first occurrence of c in s, or NULL | All |
| `l_strrchr` | Returns pointer to last occurrence of c in s, or NULL | All |
| `l_strstr` | Returns pointer to first occurrence of s2 in s1, or NULL | All |
| `l_strcmp` | Compares two strings, returns <0, 0, or >0 | All |
| `l_strncmp` | Compares up to n characters of two strings | All |
| `l_reverse` | Reverses a string in place | All |
| **Conversion functions** | | |
| `l_isdigit` | Returns non-zero if c is a digit ('0'-'9') | All |
| `l_atol` | Converts a string to a long integer | All |
| `l_atoi` | Converts a string to an integer | All |
| `l_itoa` | Converts an integer to a string in the given radix (2-36) | All |
| **Memory functions** | | |
| `l_memmove` | Copies len bytes from src to dst, handling overlapping regions | All |
| `l_memset` | Fills len bytes of dst with byte value b | All |
| `l_memcmp` | Compares n bytes of s1 and s2, returns <0, 0, or >0 | All |
| `l_memcpy` | Copies len bytes from src to dst | All |
| **System functions** | | |
| `l_exit` | Terminates the process with the given status code | All |
| `l_open` | Opens a file with the given flags and mode, returns file descriptor | All |
| `l_close` | Closes a file descriptor | All |
| `l_read` | Reads up to count bytes from fd into buf | All |
| `l_write` | Writes up to count bytes from buf to fd | All |
| `l_puts` | Writes a string to stdout | All |
| `l_exitif` | Exits with code and message if condition is true | All |
| **Convenience file openers** | | |
| `l_open_read` | Opens a file for reading | All |
| `l_open_write` | Opens or creates a file for writing | All |
| `l_open_readwrite` | Opens or creates a file for reading and writing | All |
| `l_open_append` | Opens or creates a file for appending | All |
| `l_open_trunc` | Opens or creates a file, truncating to zero length | All |
| **Terminal and timing functions (cross-platform)** | | |
| `l_sleep_ms` | Sleeps for the given number of milliseconds | All |
| `l_term_raw` | Sets stdin to raw mode (no echo, no line buffering), returns old mode | All |
| `l_term_restore` | Restores terminal mode from value returned by l_term_raw | All |
| `l_read_nonblock` | Reads from fd without blocking, returns 0 if no data available | All |
| **Unix-only functions** | | |
| `l_chdir` | Changes the current working directory | Unix |
| `l_dup` | Duplicates a file descriptor | Unix |
| `l_lseek` | Repositions the file offset of fd | Unix |
| `l_mkdir` | Creates a directory with the given permissions | Unix |
| `l_sched_yield` | Yields the processor to other threads | Unix |
| `l_sleep_ms` | Sleeps for the given number of milliseconds | Unix |
| `l_term_raw` | Sets stdin to raw mode (no echo, no line buffering), returns old mode | Unix |
| `l_term_restore` | Restores terminal mode from value returned by l_term_raw | Unix |
| `l_read_nonblock` | Reads from fd without blocking, returns 0 if no data available | Unix |

<!-- END FUNCTION REFERENCE -->

## Scope

### Not Included (by Design)
- `printf`/`sprintf` — use direct write syscalls or minimal formatting
- `malloc`/`free` — no dynamic memory allocation
- `getenv` — environment variable access
- `fork`/`exec` — process spawning and execution
- Networking functions — sockets and network I/O
- Multithreading primitives — threads, mutexes, condition variables

## Directory Structure
- `l_os.h` — Minimal C/OS abstraction header
- `test/` — Test programs for various functions
- `bin/` — Compiled test binaries
- `misc/` — Extra tools and experiments
- `Taskfile` — Bash build and test automation
- `test_all.bat` — Windows batch build and test script

## Building and Testing

### On Linux/macOS (with bash):
```sh
./Taskfile test
```

### On Windows (with Git Bash and MinGW):
```bat
test_all.bat
```

### Using ci.ps1 (PowerShell)

`ci.ps1` provides unified cross-platform CI from PowerShell. It builds, tests, and verifies across Windows, Linux (gcc + clang), and ARM (gcc + clang), delegating Linux/ARM targets to WSL.

**Parameters:**
- `-Target` — Build target: `windows`, `linux`, `arm`, or `all` (default: `all`)
- `-Action` — Action to perform: `build`, `test`, `verify`, or `all` (default: `all`)
- `-Compiler` — Compiler for Linux/ARM: `gcc`, `clang`, or `all` (default: `all` — runs both)
- `-OptLevel` — Optimization level 0–3 (default: 3)

**Examples:**
```powershell
# Build, test, and verify all targets with all compilers (default)
.\ci.ps1

# Build and test Windows only
.\ci.ps1 -Target windows -Action test

# Build Linux with clang only at -O2
.\ci.ps1 -Target linux -Action build -Compiler clang -OptLevel 2

# Test ARM binaries (via WSL + QEMU)
.\ci.ps1 -Target arm -Action test
```

**Requirements:**
- Windows builds work natively (requires clang on PATH or Visual Studio Build Tools)
- Linux and ARM builds require WSL (Windows Subsystem for Linux)
- ARM target requires `arm-linux-gnueabihf-gcc` in WSL (provides sysroot for both gcc and clang cross-compilation)
- ARM clang cross-compilation requires `clang` installed in WSL

## Adding New Tests
Add new `.c` files to the `test/` directory. They will be automatically built and run by the scripts.

## License
MIT License
