# laststanding

A freestanding C runtime — zero dependencies, direct syscalls, tiny binaries. Two header files give you everything from `strlen` to pixel graphics, across **Linux** (x86_64, ARM, AArch64) and **Windows**, with no libc at all.

| Header | What it provides |
|--------|-----------------|
| `l_os.h` | String/memory functions, file I/O, processes, pipes, terminal control, environment access |
| `l_gfx.h` | Pixel graphics — drawing primitives, bitmap font, keyboard input (Linux framebuffer / Windows GDI) |

Binaries are statically linked, stripped, and typically **2–10 KB**. The project includes 10 Unix-style utilities, 4 interactive programs (text editor, shell, snake, fractal renderer), and 5 graphical demos — all built without a single line of libc.

## Quick Start

### Hello World

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    puts("Hello from laststanding!\n");
    return 0;
}
```

```sh
gcc -I. -Oz -ffreestanding -nostdlib -static -o hello hello.c   # Linux
clang -I. -Oz -lkernel32 -ffreestanding -o hello.exe hello.c    # Windows
```

### File I/O

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    // Write a file
    L_FD fd = l_open_write("greeting.txt");
    l_write(fd, "Hello!\n", 7);
    l_close(fd);

    // Read it back
    char buf[64];
    fd = l_open_read("greeting.txt");
    ptrdiff_t n = l_read(fd, buf, sizeof(buf));
    l_close(fd);

    l_write(L_STDOUT, buf, n);
    return 0;
}
```

### Spawn a Child Process

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    char *child_argv[] = { "ls", "-l", NULL };
    L_FD pid = l_spawn("/bin/ls", child_argv, NULL);
    int code;
    l_wait(pid, &code);

    char msg[32];
    l_itoa(code, msg, 10);
    puts("Exit code: "); puts(msg); puts("\n");
    return 0;
}
```

### Pixel Graphics

```c
#define L_MAINFILE
#include "l_gfx.h"       // pulls in l_os.h automatically

int main(int argc, char *argv[]) {
    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Demo") != 0) return 1;

    l_canvas_clear(&c, L_BLACK);
    l_fill_rect(&c, 50, 50, 100, 80, L_RED);
    l_circle(&c, 200, 120, 60, L_GREEN);
    l_line(&c, 0, 0, 319, 239, L_BLUE);
    l_draw_text(&c, 60, 85, "Hello!", L_WHITE);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 'q')
        l_sleep_ms(16);
    l_canvas_close(&c);
    return 0;
}
```

### Animation Loop

```c
#define L_MAINFILE
#include "l_gfx.h"

int main(int argc, char *argv[]) {
    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Bounce") != 0) return 1;

    int x = 160, y = 120, dx = 2, dy = 1;
    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;   // ESC to quit
        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, x, y, 10, L_RED);
        l_canvas_flush(&c);
        x += dx; y += dy;
        if (x <= 10 || x >= 310) dx = -dx;
        if (y <= 10 || y >= 230) dy = -dy;
        l_sleep_ms(16);                       // ~60 fps
    }
    l_canvas_close(&c);
    return 0;
}
```

## Compile-Time Flags

| Define | Purpose |
|--------|---------|
| `L_MAINFILE` | Activates function definitions and platform startup code. **Exactly one** translation unit must define this. |
| `L_DONTOVERRIDE` | Prevents `#define strlen l_strlen` aliases — use when mixing with standard headers. |
| `L_WITHSNPRINTF` | Enables `l_snprintf` / `l_vsnprintf` (opt-in to keep binaries small). |

By default, `l_os.h` aliases standard names (`strlen`, `memcpy`, `exit`, `puts`, …) to their `l_` equivalents so you can write familiar C. Define `L_DONTOVERRIDE` to disable this.

In multi-file projects, only one `.c` file defines `L_MAINFILE`:

```c
// utils.c — gets type definitions and constants only
#include "l_os.h"

// main.c — the one file with startup code and function bodies
#define L_MAINFILE
#include "l_os.h"
```

## Key Types

| Type | Purpose |
|------|---------|
| `L_FD` | File descriptor (`ptrdiff_t`). On Windows, a library-managed slot — not a raw `HANDLE`. |
| `L_STDIN`, `L_STDOUT`, `L_STDERR` | Standard descriptor constants (0, 1, 2). |
| `L_Canvas` | Pixel graphics context (framebuffer on Linux, GDI window on Windows). |

## Compiler Flags

Binaries must be compiled freestanding:

| Platform | Command |
|----------|---------|
| Linux (gcc/clang) | `gcc -I. -Oz -ffreestanding -nostdlib -static -Wall -Wextra -Wpedantic -o app app.c` |
| Windows (clang) | `clang -I. -Oz -lkernel32 -ffreestanding -Wall -Wextra -Wpedantic -o app.exe app.c` |
| ARM32 cross | `arm-linux-gnueabihf-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |
| AArch64 cross | `aarch64-linux-gnu-gcc -I. -Oz -ffreestanding -nostdlib -static -o app app.c` |

## Function Reference — `l_os.h`

Generated from doc-comments. Run `.\gen-docs.ps1` to regenerate.

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
| `l_strtoul` | Converts a string to an unsigned long, auto-detecting base (0x=hex, 0=octal) | All |
| `l_strtol` | Converts a string to a long, auto-detecting base; handles leading sign | All |
| `l_itoa` | Converts an integer to a string in the given radix (2-36) | All |
| **Memory functions** | | |
| `l_memmove` | Copies len bytes from src to dst, handling overlapping regions | All |
| `l_memset` | Fills len bytes of dst with byte value b | All |
| `l_memcmp` | Compares n bytes of s1 and s2, returns <0, 0, or >0 | All |
| `l_memcpy` | Copies len bytes from src to dst | All |
| `l_memchr` | Finds first occurrence of byte c in the first n bytes of s, or NULL | All |
| `l_memrchr` | Finds last occurrence of byte c in the first n bytes of s, or NULL | All |
| `l_strnlen` | Returns the length of s, but at most maxlen | All |
| `l_memmem` | Finds first occurrence of needle in haystack, or NULL | All |
| **Formatted output** (opt-in: `#define L_WITHSNPRINTF`) | | |
| `l_vsnprintf` | `va_list` version of `l_snprintf` | All |
| `l_snprintf` | Formats a string into buf (at most n bytes including NUL) | All |
| **System functions** | | |
| `l_exit` | Terminates the process with the given status code | All |
| `l_open` | Opens a file with the given flags and mode, returns file descriptor | All |
| `l_close` | Closes a file descriptor | All |
| `l_read` | Reads up to count bytes from fd into buf | All |
| `l_write` | Writes up to count bytes from buf to fd | All |
| `l_puts` | Writes a string to stdout | All |
| `l_exitif` | Exits with code and message if condition is true | All |
| `l_getenv` | Returns value of environment variable, or NULL if not found | All |
| `l_find_executable` | Searches PATH for an executable, returns 1 if found | All |
| **Convenience file openers** | | |
| `l_open_read` | Opens a file for reading | All |
| `l_open_write` | Opens or creates a file for writing | All |
| `l_open_readwrite` | Opens or creates a file for reading and writing | All |
| `l_open_append` | Opens or creates a file for appending | All |
| `l_open_trunc` | Opens or creates a file, truncating to zero length | All |
| **Terminal and timing** | | |
| `l_sleep_ms` | Sleeps for the given number of milliseconds | All |
| `l_term_raw` | Sets stdin to raw mode, returns old mode for later restore | All |
| `l_term_restore` | Restores terminal mode from value returned by `l_term_raw` | All |
| `l_read_nonblock` | Reads from fd without blocking, returns 0 if no data | All |
| `l_term_size` | Gets terminal size in rows and columns | All |
| **File system** | | |
| `l_unlink` | Deletes a file | All |
| `l_rmdir` | Removes an empty directory | All |
| `l_rename` | Renames a file or directory | All |
| `l_access` | Checks file access (`L_F_OK`, `L_R_OK`, `L_W_OK`, `L_X_OK`) | All |
| `l_stat` / `l_fstat` | Gets file metadata (size, mode, timestamps) by path or fd | All |
| `l_opendir` / `l_readdir` / `l_closedir` | Directory iteration | All |
| `l_mmap` / `l_munmap` | Memory-mapped I/O | All |
| `l_getcwd` / `l_chdir` | Working directory | All |
| **Pipes and processes** | | |
| `l_pipe` | Creates a pipe (fds[0]=read, fds[1]=write) | All |
| `l_dup` / `l_dup2` | Duplicate file descriptors | All |
| `l_spawn` | Spawns a child process | All |
| `l_spawn_stdio` | Spawns with explicit stdin/stdout/stderr redirection | All |
| `l_wait` | Waits for child process, returns exit code | All |
| **Unix-only** | | |
| `l_lseek` | Repositions file offset | Unix |
| `l_mkdir` | Creates a directory | Unix |
| `l_fork` / `l_execve` / `l_waitpid` | Traditional Unix process control | Unix |

<!-- END FUNCTION REFERENCE -->

## Function Reference — `l_gfx.h`

| Function | Description |
|----------|-------------|
| **Canvas lifecycle** | |
| `l_canvas_open(canvas, w, h, title)` | Open a window/framebuffer. Returns 0 on success. |
| `l_canvas_close(canvas)` | Close and free resources |
| `l_canvas_alive(canvas)` | Returns non-zero while the window is open |
| `l_canvas_flush(canvas)` | Blit the pixel buffer to screen |
| `l_canvas_clear(canvas, color)` | Fill the entire canvas with a color |
| `l_canvas_key(canvas)` | Non-blocking key read (0 if none, ASCII code or ESC=27) |
| **Drawing primitives** | |
| `l_pixel(canvas, x, y, color)` | Set a single pixel |
| `l_get_pixel(canvas, x, y)` | Read a pixel's color |
| `l_line(canvas, x0, y0, x1, y1, color)` | Bresenham line |
| `l_rect(canvas, x, y, w, h, color)` | Outline rectangle |
| `l_fill_rect(canvas, x, y, w, h, color)` | Filled rectangle |
| `l_circle(canvas, cx, cy, r, color)` | Outline circle (midpoint algorithm) |
| `l_fill_circle(canvas, cx, cy, r, color)` | Filled circle |
| **Text** | |
| `l_draw_char(canvas, x, y, ch, color)` | Draw one character (8×8 bitmap font) |
| `l_draw_text(canvas, x, y, str, color)` | Draw a string |
| **Colors** | |
| `L_RGB(r, g, b)` | Compose a 32-bit ARGB color |
| `L_BLACK`, `L_WHITE`, `L_RED`, `L_GREEN`, `L_BLUE` | Predefined constants |

**Platform backends:**
- **Linux:** renders to `/dev/fb0` (framebuffer console — no X11 or Wayland)
- **Windows:** native GDI window (`user32.dll` + `gdi32.dll`)

All graphical demos use **integer-only math** (no floats) for full ARM compatibility.

## Example Programs

Every program in `test/` compiles to a small, self-contained binary with no libc dependency.

### Utilities

| Program | Description | Source |
|---------|-------------|--------|
| **base64** | Base64 encoder/decoder (RFC 4648) | [base64.c](test/base64.c) |
| **checksum** | XOR and additive file checksums | [checksum.c](test/checksum.c) |
| **countlines** | Line counter | [countlines.c](test/countlines.c) |
| **grep** | Substring pattern filter | [grep.c](test/grep.c) |
| **hexdump** | Hex + ASCII file dump | [hexdump.c](test/hexdump.c) |
| **ls** | Directory listing (`-a`, `-l`) | [ls.c](test/ls.c) |
| **printenv** | Print environment variables | [printenv.c](test/printenv.c) |
| **sort** | Line sort (`-r`, `-f`, `-n`, `-u`) | [sort.c](test/sort.c) |
| **upper** | Uppercase filter | [upper.c](test/upper.c) |
| **wc** | Line/word/byte counter | [wc.c](test/wc.c) |

### Interactive Programs

| Program | Description | Source |
|---------|-------------|--------|
| **led** | Vim-style text editor (hjkl, insert/normal/command, :w/:q, search) | [led.c](test/led.c) |
| **mandelbrot** | Fixed-point fractal renderer — pan, zoom, iteration control | [mandelbrot.c](test/mandelbrot.c) |
| **sh** | Shell — builtins, PATH search, quotes, I/O redirection, pipes | [sh.c](test/sh.c) |
| **snake** | Terminal Snake game with WASD controls | [snake.c](test/snake.c) |

### Graphical Demos (`l_gfx.h`)

| Program | Description | Source |
|---------|-------------|--------|
| **life** | Conway's Game of Life — 80×60 grid, pause/randomize/clear | [life.c](test/life.c) |
| **plasma** | Rainbow plasma — animated sine-wave color cycling | [plasma.c](test/plasma.c) |
| **starfield** | 3D starfield — 200 stars with perspective projection | [starfield.c](test/starfield.c) |
| **fire** | Doom-style fire — bottom-up heat propagation | [fire.c](test/fire.c) |
| **clock** | Analog clock — hour/minute/second hands, ticking in real time | [clock.c](test/clock.c) |

### Test Suite

| Program | Assertions | Source |
|---------|-----------|--------|
| **test** | 572 (Linux/ARM/AArch64), 561 (Windows) | [test.c](test/test.c) |
| **gfx_test** | 28 (in-memory pixel buffer tests) | [gfx_test.c](test/gfx_test.c) |

## Directory Structure

```
l_os.h          — Core runtime header (strings, I/O, processes, terminal)
l_gfx.h        — Pixel graphics header (drawing, font, canvas)
test/           — All example programs and tests
bin/            — Compiled binaries (generated)
misc/           — Reference implementations using standard libc
Taskfile        — Linux/macOS build automation (bash)
build.bat       — Windows build script
test_all.bat    — Windows build + test
ci.ps1          — Cross-platform CI (PowerShell)
```

## Building and Testing

### Linux/macOS
```sh
./Taskfile test              # build + run all tests
./Taskfile build clang 2     # build with clang at -O2
```

### Windows
```bat
test_all.bat                 :: build + run all tests
```

### Full CI (all platforms from Windows via WSL)

```powershell
.\ci.ps1                                              # all targets, all compilers
.\ci.ps1 -Target windows -Action test                 # Windows only
.\ci.ps1 -Target linux -Compiler clang -OptLevel 2    # Linux clang -O2
.\ci.ps1 -Target arm -Action test                     # ARM32 + AArch64 via QEMU
```

`ci.ps1` builds, tests, and verifies across **7 configurations**: Windows, Linux gcc, Linux clang, ARM gcc, ARM clang, AArch64 gcc, AArch64 clang. Linux and ARM targets run in WSL; ARM/AArch64 binaries execute via QEMU user-mode emulation.

**Requirements:** clang on PATH (Windows), WSL with `gcc`, `clang`, `arm-linux-gnueabihf-gcc`, `aarch64-linux-gnu-gcc`, and `qemu-user` (Linux/ARM).

## Scope — Not Included (by Design)
- `printf`/`sprintf` — use `l_snprintf` or direct `l_write`
- `malloc`/`free` — no heap allocation
- Networking — no sockets
- Threads — no multithreading primitives

## License
MIT License
