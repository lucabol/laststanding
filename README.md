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
gcc -I. -Oz -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o myapp myapp.c
```

**Windows (clang):**
```bat
clang -I. -Oz -lkernel32 -ffreestanding -Wall -Wextra -Wpedantic -o myapp.exe myapp.c
```

**ARM cross-compilation (gcc):**
```sh
arm-linux-gnueabihf-gcc -I. -Oz -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o myapp myapp.c
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
| `l_strcat` | Appends src string to dst, returns dst | All |
| `l_strncat` | Appends at most n characters of src to dst, always null-terminates, returns dst | All |
| `l_strchr` | Returns pointer to first occurrence of c in s, or NULL | All |
| `l_strrchr` | Returns pointer to last occurrence of c in s, or NULL | All |
| `l_strstr` | Returns pointer to first occurrence of s2 in s1, or NULL | All |
| `l_strcmp` | Compares two strings, returns <0, 0, or >0 | All |
| `l_strncmp` | Compares up to n characters of two strings | All |
| `l_strcasecmp` | Case-insensitive string comparison | All |
| `l_strncasecmp` | Case-insensitive comparison of up to n characters | All |
| `l_strspn` | Returns length of initial segment of s consisting entirely of bytes in accept | All |
| `l_strcspn` | Returns length of initial segment of s consisting entirely of bytes NOT in reject | All |
| `l_basename` | Returns pointer to the filename component of path (after last '/' or '\') | All |
| `l_dirname` | Writes the directory component of path into buf (up to bufsize), returns buf | All |
| `l_reverse` | Reverses a string in place | All |
| **Conversion functions** | | |
| `l_isspace` | Returns non-zero if c is a whitespace character (space, tab, newline, etc.) | All |
| `l_isdigit` | Returns non-zero if c is a digit ('0'-'9') | All |
| `l_isalpha` | Returns non-zero if c is an alphabetic character ('A'-'Z' or 'a'-'z') | All |
| `l_isalnum` | Returns non-zero if c is alphanumeric (l_isalpha or l_isdigit) | All |
| `l_isupper` | Returns non-zero if c is an uppercase letter ('A'-'Z') | All |
| `l_islower` | Returns non-zero if c is a lowercase letter ('a'-'z') | All |
| `l_toupper` | Converts c to uppercase; returns c unchanged if not a lowercase letter | All |
| `l_tolower` | Converts c to lowercase; returns c unchanged if not an uppercase letter | All |
| `l_atol` | Converts a string to a long integer, skipping leading whitespace | All |
| `l_atoi` | Converts a string to an integer | All |
| `l_strtoul` | Converts a string to an unsigned long, auto-detecting base when base==0 (0x=hex, 0=octal, else decimal); sets *endptr past last digit | All |
| `l_strtol` | Converts a string to a long, auto-detecting base when base==0; handles leading sign; sets *endptr past last digit | All |
| `l_itoa` | Converts an integer to a string in the given radix (2-36) | All |
| **Memory functions** | | |
| `l_memmove` | Copies len bytes from src to dst, handling overlapping regions | All |
| `l_memset` | Fills len bytes of dst with byte value b | All |
| `l_memcmp` | Compares n bytes of s1 and s2, returns <0, 0, or >0 | All |
| `l_memcpy` | Copies len bytes from src to dst | All |
| `l_memchr` | Finds first occurrence of byte c in the first n bytes of s, or NULL | All |
| `l_memrchr` | Finds last occurrence of byte c in the first n bytes of s, or NULL | All |
| `l_strnlen` | Returns the length of s, but at most maxlen (does not scan past maxlen bytes) | All |
| `l_memmem` | Finds first occurrence of needle (needlelen bytes) in haystack (haystacklen bytes), or NULL | All |
| **Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)** | | |
| `l_vsnprintf` | Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written | All |
| `l_snprintf` | Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written | All |
| **System functions** | | |
| `l_exit` | Terminates the process with the given status code | All |
| `l_open` | Opens a file with the given flags and mode, returns file descriptor | All |
| `l_close` | Closes a file descriptor | All |
| `l_read` | Reads up to count bytes from fd into buf | All |
| `l_write` | Writes up to count bytes from buf to fd | All |
| `l_puts` | Writes a string to stdout | All |
| `l_exitif` | Exits with code and message if condition is true | All |
| `l_getenv` | Returns value of environment variable, or NULL if not found | All |
| `l_getenv_init` | Initializes environment variable access (call from main) | All |
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
| `l_term_size` | Gets terminal size in rows and columns | All |
| **File system functions (cross-platform)** | | |
| `l_unlink` | Deletes a file, returns 0 on success, -1 on error | All |
| `l_rename` | Renames (or moves) a file or directory. Returns 0 on success, -1 on error. | All |
| `l_access` | Checks access to a file. mode: L_F_OK (exists), L_R_OK, L_W_OK, L_X_OK. Returns 0 if ok, -1 on error. | All |
| `l_rmdir` | Removes an empty directory, returns 0 on success, -1 on error | All |
| `l_stat` | Gets file metadata by path. Returns 0 on success, -1 on error. | All |
| `l_fstat` | Gets file metadata by open file descriptor. Returns 0 on success, -1 on error. | All |
| `l_opendir` | Opens a directory for reading. Returns 0 on success, -1 on error. | All |
| `l_readdir` | Reads the next directory entry. Returns pointer to L_DirEntry or NULL when done. | All |
| `l_closedir` | Closes a directory handle. | All |
| `l_mmap` | Maps a file or anonymous memory into the process address space | All |
| `l_munmap` | Unmaps a previously mapped region | All |
| `l_getcwd` | Gets the current working directory into buf (up to size bytes). Returns buf on success, NULL on error. | All |
| `l_chdir` | Changes the current working directory | All |
| `l_pipe` | Creates a pipe. fds[0] is the read end, fds[1] is the write end. Returns 0 on success, -1 on error. | All |
| `l_dup2` | Duplicates oldfd onto newfd. Returns newfd on success, -1 on error. | All |
| `l_spawn` | path: executable path. argv: NULL-terminated argument array. envp: NULL-terminated environment (NULL = inherit). | All |
| `l_wait` | exitcode receives the process exit code. | All |
| **Unix-only functions** | | |
| `l_dup` | Duplicates a file descriptor | Unix |
| `l_lseek` | Repositions the file offset of fd | Unix |
| `l_mkdir` | Creates a directory with the given permissions | Unix |
| `l_sched_yield` | Yields the processor to other threads | Unix |
| `l_fork` | Fork the current process. Returns child pid to parent, 0 to child, -1 on error. | Unix |
| `l_execve` | Replace the current process image. Does not return on success. | Unix |
| `l_waitpid` | Wait for a child process. Returns child pid on success, -1 on error. | Unix |

<!-- END FUNCTION REFERENCE -->

## Scope

### Not Included (by Design)
- `printf`/`sprintf` — use direct write syscalls or `l_snprintf`
- `malloc`/`free` — no dynamic memory allocation
- Networking functions — sockets and network I/O
- Multithreading primitives — threads, mutexes, condition variables

## Example Programs

The `test/` directory contains example programs that showcase `l_os.h` capabilities. Each compiles to a small, self-contained binary with no libc dependency.

### Utilities

| Program | Description | Source |
|---------|-------------|--------|
| **countlines** | Counts lines in a file | [countlines.c](test/countlines.c) |
| **grep** | Filters lines matching a substring pattern | [grep.c](test/grep.c) |
| **hexdump** | Displays file contents in hex + ASCII format | [hexdump.c](test/hexdump.c) |
| **checksum** | Computes XOR and additive checksums of a file | [checksum.c](test/checksum.c) |
| **upper** | Converts text to uppercase | [upper.c](test/upper.c) |
| **wc** | Counts lines, words, and bytes | [wc.c](test/wc.c) |
| **printenv** | Prints environment variables (all or by name) | [printenv.c](test/printenv.c) |

### Interactive Programs

| Program | Description | Source |
|---------|-------------|--------|
| **led** | Modal text editor with vim keybindings (hjkl, insert/normal/command modes, :w/:q, search) | [led.c](test/led.c) |
| **snake** | Playable Snake console game with WASD controls and ANSI rendering | [snake.c](test/snake.c) |

### Test Suite

| Program | Description | Source |
|---------|-------------|--------|
| **test** | Comprehensive test suite (200+ assertions covering all `l_` functions) | [test.c](test/test.c) |

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
