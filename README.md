# laststanding

A freestanding C runtime — zero dependencies, direct syscalls, tiny binaries. Three header files give you everything from `strlen` to pixel graphics to interactive UI widgets, across **Linux** (x86_64, ARM, AArch64) and **Windows**, with no libc at all.

| Header | What it provides |
|--------|-----------------|
| `l_os.h` | String/memory functions, file I/O, processes, pipes, terminal control, environment access |
| `l_gfx.h` | Pixel graphics — drawing primitives, bitmap font, keyboard/mouse input (Linux framebuffer / Windows GDI) |
| `l_ui.h` | Immediate-mode UI — buttons, checkboxes, sliders, text inputs, layout helpers (built on `l_gfx.h`) |

Binaries are statically linked, stripped, and typically **2–10 KB**. The project includes 10 Unix-style utilities, 4 interactive programs (text editor, shell, snake, fractal renderer), 5 graphical demos, and 2 UI demos — all built without a single line of libc.

## Quick Start

### Hello World (`l_os.h`)

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

### File I/O (`l_os.h`)

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    L_FD fd = l_open_write("greeting.txt");
    l_write(fd, "Hello!\n", 7);
    l_close(fd);

    char buf[64];
    fd = l_open_read("greeting.txt");
    ptrdiff_t n = l_read(fd, buf, sizeof(buf));
    l_close(fd);

    l_write(L_STDOUT, buf, n);
    return 0;
}
```

### Spawn a Child Process (`l_os.h`)

```c
#define L_MAINFILE
#include "l_os.h"

int main(int argc, char *argv[]) {
    // Run "upper" from the same bin/ directory
    char *child_argv[] = { "upper", NULL };
    L_FD pid = l_spawn("bin/upper", child_argv, NULL);
    int code;
    l_wait(pid, &code);

    char msg[32];
    l_itoa(code, msg, 10);
    puts("Exit code: "); puts(msg); puts("\n");
    return 0;
}
```

### Pixel Graphics (`l_gfx.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"       // pulls in l_os.h automatically

int main(int argc, char *argv[]) {
    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Demo") != 0) return 1;

    l_canvas_clear(&c, L_BLACK);
    l_fill_rect(&c, 50, 50, 100, 80, L_RED);
    l_circle(&c, 200, 120, 60, L_GREEN);
    l_draw_text(&c, 60, 85, "Hello!", L_WHITE);
    l_canvas_flush(&c);

    while (l_canvas_alive(&c) && l_canvas_key(&c) != 27)
        l_sleep_ms(16);
    l_canvas_close(&c);
    return 0;
}
```

### Animation Loop (`l_gfx.h`)

```c
#define L_MAINFILE
#include "l_gfx.h"

int main(int argc, char *argv[]) {
    L_Canvas c;
    if (l_canvas_open(&c, 320, 240, "Bounce") != 0) return 1;

    int x = 160, y = 120, dx = 2, dy = 1;
    while (l_canvas_alive(&c)) {
        if (l_canvas_key(&c) == 27) break;
        l_canvas_clear(&c, L_BLACK);
        l_fill_circle(&c, x, y, 10, L_RED);
        l_canvas_flush(&c);
        x += dx; y += dy;
        if (x <= 10 || x >= 310) dx = -dx;
        if (y <= 10 || y >= 230) dy = -dy;
        l_sleep_ms(16);
    }
    l_canvas_close(&c);
    return 0;
}
```

### UI Form (`l_ui.h`)

```c
#define L_MAINFILE
#include "l_ui.h"         // pulls in l_gfx.h and l_os.h automatically

int main(int argc, char *argv[]) {
    L_Canvas canvas;
    if (l_canvas_open(&canvas, 400, 300, "My App") != 0) return 1;

    L_UI ui;
    l_ui_init(&ui);

    int clicked = 0, checked = 0, volume = 50;
    char name[256] = "";

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(30, 30, 34));
        if (ui.key == 27) break;

        l_ui_panel(&ui, 10, 10, 380, 280);

        l_ui_column_begin(&ui, 20, 20, 8);

        int y = l_ui_next(&ui, 10);
        l_ui_label(&ui, 20, y, "Settings");

        y = l_ui_next(&ui, 24);
        if (l_ui_button(&ui, 20, y, 120, 24, "Click Me"))
            clicked++;

        y = l_ui_next(&ui, 16);
        l_ui_checkbox(&ui, 20, y, "Dark mode", &checked);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Volume:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &volume, 0, 100);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Name:");
        y = l_ui_next(&ui, 20);
        l_ui_textbox(&ui, 20, y, 200, name, 256);

        l_ui_layout_end(&ui);

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }
    l_canvas_close(&canvas);
    return 0;
}
```

### RGB Color Mixer (`l_ui.h`)

```c
#define L_MAINFILE
#include "l_ui.h"

int main(int argc, char *argv[]) {
    L_Canvas canvas;
    if (l_canvas_open(&canvas, 480, 300, "Color Mixer") != 0) return 1;

    L_UI ui;
    l_ui_init(&ui);
    int r = 128, g = 64, b = 200;

    while (l_canvas_alive(&canvas)) {
        l_ui_begin(&ui, &canvas);
        l_canvas_clear(&canvas, L_RGB(25, 25, 28));
        if (ui.key == 27) break;

        l_ui_column_begin(&ui, 20, 20, 8);

        int y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Red:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &r, 0, 255);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Green:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &g, 0, 255);

        y = l_ui_next(&ui, 8);
        l_ui_label(&ui, 20, y, "Blue:");
        y = l_ui_next(&ui, 16);
        l_ui_slider(&ui, 20, y, 200, &b, 0, 255);

        l_ui_layout_end(&ui);

        // Color preview
        l_fill_rect(&canvas, 250, 20, 210, 260, L_RGB(r, g, b));

        l_ui_end(&ui);
        l_canvas_flush(&canvas);
    }
    l_canvas_close(&canvas);
    return 0;
}
```

### Dynamic Memory (Arena Allocator)

laststanding provides an arena (bump) allocator for programs that need dynamic memory without the complexity of malloc/free:

```c
L_Arena arena = l_arena_init(1024 * 1024);  // 1MB arena
char *name = l_arena_alloc(&arena, 256);     // bump-allocate 256 bytes
l_arena_reset(&arena);                       // reuse all memory
l_arena_free(&arena);                        // release to OS

L_Buf buf;
l_buf_init(&buf);
l_buf_push(&buf, "hello", 5);               // growable buffer
l_buf_printf(&buf, " world %d", 42);        // formatted append (needs L_WITHSNPRINTF)
l_buf_free(&buf);
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
| `L_UI` | Immediate-mode UI context (theme, input state, layout cursor). |
| `L_UI_Theme` | UI color theme (background, hover, active, text, border, accent, input). |

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
| `l_strpbrk` | Returns pointer to first occurrence in s of any character in accept, or NULL | All |
| `l_strtok_r` | Splits str into tokens delimited by any char in delim; saves state in *saveptr (reentrant) | All |
| `l_basename` | Returns pointer to the filename component of path (after last '/' or '\') | All |
| `l_dirname` | Writes the directory component of path into buf (up to bufsize), returns buf | All |
| `l_path_join` | Joins directory and filename with separator, returns buf | All |
| `l_path_ext` | Returns pointer to extension including dot (".txt"), or "" if none | All |
| `l_path_exists` | Returns 1 if path exists, 0 if not | All |
| `l_path_isdir` | Returns 1 if path is a directory, 0 if not | All |
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
| `l_isprint` | Returns non-zero if c is a printable ASCII character (0x20-0x7e) | All |
| `l_isxdigit` | Returns non-zero if c is a hexadecimal digit (0-9, a-f, A-F) | All |
| `l_abs` | Returns the absolute value of an integer | All |
| `l_labs` | Returns the absolute value of a long | All |
| `l_llabs` | Returns the absolute value of a long long | All |
| `l_atol` | Converts a string to a long integer, skipping leading whitespace | All |
| `l_atoi` | Converts a string to an integer | All |
| `l_strtoul` | Converts a string to an unsigned long, auto-detecting base when base==0 (0x=hex, 0=octal, else decimal); sets *endptr past last digit | All |
| `l_strtol` | Converts a string to a long, auto-detecting base when base==0; handles leading sign; sets *endptr past last digit | All |
| `l_strtoull` | Converts a string to an unsigned long long (64-bit); auto-detects base when base==0; sets *endptr past last digit | All |
| `l_strtoll` | Converts a string to a long long (64-bit); auto-detects base when base==0; handles leading sign; sets *endptr past last digit | All |
| `l_strtod` | Converts a string to a double; skips leading whitespace; handles sign, decimal point, and e/E exponent; sets *endptr past last digit | All |
| `l_atof` | Converts a string to a double (convenience wrapper around l_strtod) | All |
| **Math functions** | | |
| `l_fabs` | Returns the absolute value of a double | All |
| `l_floor` | Rounds toward negative infinity | All |
| `l_ceil` | Rounds toward positive infinity | All |
| `l_fmod` | Floating-point remainder of x/y | All |
| `l_sqrt` | Square root via Newton-Raphson with IEEE bit hack seed | All |
| `l_sin` | Sine via range reduction and Taylor series | All |
| `l_cos` | Cosine: l_sin(x + pi/2) | All |
| `l_exp` | Exponential function via range reduction and Taylor series | All |
| `l_log` | Natural logarithm via mantissa/exponent decomposition | All |
| `l_pow` | Power: base^exp via exp(exp * log(base)) | All |
| `l_atan2` | Two-argument arctangent with quadrant handling | All |
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
| **Random number generation (xorshift32, single-threaded)** | | |
| `l_srand` | Seeds the pseudo-random number generator | All |
| `l_rand` | Returns a pseudo-random unsigned int (xorshift32) | All |
| **Formatted output (opt-in: define L_WITHSNPRINTF before including l_os.h)** | | |
| `l_vsnprintf` | Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written | All |
| `l_snprintf` | Formats a string into buf (at most n bytes including NUL); returns number of chars that would have been written | All |
| `l_dprintf` | Writes formatted output to file descriptor fd. Returns number of bytes written. | All |
| **System functions** | | |
| `l_exit` | Terminates the process with the given status code | All |
| `l_open` | Opens a file with the given flags and mode, returns file descriptor | All |
| `l_close` | Closes a file descriptor | All |
| `l_read` | Reads up to count bytes from fd into buf | All |
| `l_write` | Writes up to count bytes from buf to fd | All |
| `l_read_line` | Reads one line from fd into buf (up to bufsz-1 bytes). Strips the newline. | All |
| `l_time` | Returns current Unix timestamp (seconds since 1970-01-01). Also writes to *t if non-NULL. | All |
| `l_puts` | Writes a string to stdout | All |
| `l_exitif` | Exits with code and message if condition is true | All |
| `l_getenv` | Returns value of environment variable, or NULL if not found | All |
| `l_getenv_init` | Initializes environment variable access (call from main) | All |
| `l_env_start` | Begin iterating environment variables. Returns opaque handle (pass to l_env_end). | All |
| `l_env_next` | Get next "KEY=VALUE" string. buf/bufsz provide conversion space (Windows). | All |
| `l_env_end` | End iteration and free resources. | All |
| `l_find_executable` | Finds an executable by name, searching PATH if needed. | All |
| **Option parsing (single-threaded; state in static variables)** | | |
| `l_getopt` | Parses command-line options. optstring lists valid option chars; trailing ':' means the option | All |
| **Convenience file openers** | | |
| `l_open_read` | Opens a file for reading | All |
| `l_open_write` | Opens or creates a file for writing | All |
| `l_open_readwrite` | Opens or creates a file for reading and writing | All |
| `l_open_append` | Opens or creates a file for appending | All |
| `l_open_trunc` | Opens or creates a file, truncating to zero length | All |
| **Error reporting** | | |
| `l_errno` | Returns the error code from the most recent failed syscall (0 if last call succeeded) | All |
| `l_strerror` | Returns a human-readable string for the given error code | All |
| **Terminal and timing functions (cross-platform)** | | |
| `l_sleep_ms` | Sleeps for the given number of milliseconds | All |
| `l_term_raw` | Sets stdin to raw mode (no echo, no line buffering), returns old mode | All |
| `l_term_restore` | Restores terminal mode from value returned by l_term_raw | All |
| `l_read_nonblock` | Reads from fd without blocking, returns 0 if no data available | All |
| `l_term_size` | Gets terminal size in rows and columns | All |
| **ANSI terminal helpers** | | |
| `l_ansi_move` | Writes cursor-move sequence into buf, returns bytes written | All |
| `l_ansi_color` | Writes color sequence into buf; fg/bg are 0-7 ANSI colors, -1 for default | All |
| **File system functions (cross-platform)** | | |
| `l_unlink` | Deletes a file, returns 0 on success, -1 on error | All |
| `l_rmdir` | Removes an empty directory, returns 0 on success, -1 on error | All |
| `l_rename` | Renames (or moves) a file or directory. Returns 0 on success, -1 on error. | All |
| `l_access` | Checks access to a file. mode: L_F_OK (exists), L_R_OK, L_W_OK, L_X_OK. Returns 0 if ok, -1 on error. | All |
| `l_chmod` | Changes permission bits of a file. Returns 0 on success, -1 on error. | All |
| `l_symlink` | Creates a symbolic link at linkpath pointing to target. Returns 0 on success, -1 on error. | All |
| `l_readlink` | Reads the target of a symbolic link into buf (up to bufsiz bytes). Returns number of bytes read, or -1 on error. | All |
| `l_realpath` | Resolves path to its canonical absolute form into resolved (at least L_PATH_MAX bytes). Returns resolved on success, NULL on error. | All |
| `l_stat` | Gets file metadata by path. Returns 0 on success, -1 on error. | All |
| `l_fstat` | Gets file metadata by open file descriptor. Returns 0 on success, -1 on error. | All |
| `l_opendir` | Opens a directory for reading. Returns 0 on success, -1 on error. | All |
| `l_readdir` | Reads the next directory entry. Returns pointer to L_DirEntry or NULL when done. | All |
| `l_closedir` | Closes a directory handle. | All |
| `l_mmap` | Maps a file or anonymous memory into the process address space | All |
| `l_munmap` | Unmaps a previously mapped region | All |
| **Arena function declarations** | | |
| `l_arena_init` | Allocate an arena of `size` bytes via mmap. On failure, base=NULL. | All |
| `l_arena_alloc` | Bump-allocate n bytes (8-byte aligned). Returns NULL if arena is full. | All |
| `l_arena_reset` | Reset used to 0. Memory is NOT freed — arena can be reused. | All |
| `l_arena_free` | Free the backing memory. Sets base=NULL. | All |
| **Buffer function declarations** | | |
| `l_buf_init` | Zero-initialize a buffer. | All |
| `l_buf_push` | Append n bytes. Returns 0 on success, -1 on failure. | All |
| `l_buf_printf` | Formatted append using l_vsnprintf. Returns bytes written or -1. | All |
| `l_buf_clear` | Set len=0 (keep allocated memory). | All |
| `l_buf_free` | Free backing memory and zero the struct. | All |
| `l_getcwd` | Gets the current working directory into buf (up to size bytes). Returns buf on success, NULL on error. | All |
| `l_chdir` | Changes the current working directory | All |
| `l_pipe` | Creates a pipe. fds[0] is the read end, fds[1] is the write end. Returns 0 on success, -1 on error. | All |
| `l_dup` | Duplicates fd, returning a new descriptor on success or -1 on error. | All |
| `l_dup2` | Duplicates oldfd onto newfd. Returns newfd on success, -1 on error. | All |
| `l_getpid` | Returns the current process ID. | All |
| `l_spawn_stdio` | Spawns a new process with explicit stdio. Use L_SPAWN_INHERIT to keep the parent's stream. | All |
| `l_spawn` | Spawns a new process, inheriting the current stdio descriptors. | All |
| `l_wait` | Waits for a spawned process to finish. Returns 0 on success, -1 on error. | All |
| **Unix-only functions** | | |
| `l_lseek` | Repositions the file offset of fd | Unix |
| `l_mkdir` | Creates a directory with the given permissions | Unix |
| `l_sched_yield` | Yields the processor to other threads | Unix |
| `l_fork` | Fork the current process. Returns child pid to parent, 0 to child, -1 on error. | Unix |
| `l_execve` | Replace the current process image. Does not return on success. | Unix |
| `l_waitpid` | Wait for a child process. Returns child pid on success, -1 on error. | Unix |
| `l_getppid` | Returns the parent process ID. | Unix |
| `l_kill` | Sends signal sig to process pid. Returns 0 on success, -1 on error. | Unix |
| **Byte order helpers** | | |
| `l_htons` | Convert 16-bit value from host to network byte order | All |
| `l_htonl` | Convert 32-bit value from host to network byte order | All |
| `l_inet_addr` | Parse dotted-quad IP string to network-order u32. Returns 0 on error. | All |
| **TCP socket functions** | | |
| `l_socket_tcp` | Create a TCP socket. Returns socket fd or -1 on error. | All |
| `l_socket_connect` | Connect to addr:port. Returns 0 on success, -1 on error. | All |
| `l_socket_bind` | Bind socket to port on all interfaces. Returns 0/-1. | All |
| `l_socket_listen` | Listen for connections. Returns 0/-1. | All |
| `l_socket_accept` | Accept connection. Returns new socket or -1. | All |
| `l_socket_send` | Send data. Returns bytes sent or -1. | All |
| `l_socket_recv` | Receive data. Returns bytes received, 0 on close, -1 on error. | All |
| `l_socket_close` | Close socket. | All |

<!-- END FUNCTION REFERENCE -->

## Function Reference — `l_gfx.h`

Generated from doc-comments. Run `.\gen-docs.ps1` to regenerate.

<!-- BEGIN GFX REFERENCE -->

| Function | Description |
|----------|-------------|
| **Color helpers** | |
| `L_RGB` | Composes a 32-bit ARGB color from red, green, blue (0-255). |
| `L_RGBA` | Composes a 32-bit ARGB color from red, green, blue, alpha (0-255). |
| **API declarations** | |
| `l_canvas_open` | Opens a canvas. Returns 0 on success, -1 on error (e.g. no display). |
| `l_canvas_close` | Closes the canvas and frees resources. |
| `l_canvas_alive` | Returns non-zero if the canvas is still alive (window not closed). |
| `l_canvas_flush` | Copies the pixel buffer to the screen. |
| `l_canvas_clear` | Fills the entire pixel buffer with a single color. |
| `l_canvas_key` | Returns the next key press (ASCII or arrow codes), or 0 if none. Non-blocking. |
| `l_canvas_mouse` | Returns mouse button bitmask (1=left, 2=right, 4=middle) and writes position to *x, *y. |
| **Drawing primitives (platform-independent, operate on pixels[])** | |
| `l_pixel` | Sets a single pixel at (x, y) to the given color. No-op if out of bounds. |
| `l_get_pixel` | Returns the color of the pixel at (x, y), or 0 if out of bounds. |
| `l_line` | Draws a line from (x0,y0) to (x1,y1) using Bresenham's algorithm. |
| `l_rect` | Draws an outline rectangle at (x,y) with width w and height h. |
| `l_fill_rect` | Draws a filled rectangle at (x,y) with width w and height h. |
| `l_circle` | Draws an outline circle centered at (cx,cy) with radius r (midpoint algorithm). |
| `l_fill_circle` | Draws a filled circle centered at (cx,cy) with radius r. |
| `l_hline` | Draws a horizontal line from x0 to x1 at row y (used internally by fill_circle). |
| **Text rendering** | |
| `l_draw_char` | Draws a single character at (x,y) using the embedded 8x8 bitmap font. |
| `l_draw_text` | Draws a null-terminated string at (x,y), advancing 8 pixels per character. |

<!-- END GFX REFERENCE -->

Platform backends: **Linux** renders to `/dev/fb0` (framebuffer console — no X11 or Wayland). You may need to grant access first: `sudo chmod 666 /dev/fb0`. **Windows** opens a native GDI window (`user32.dll` + `gdi32.dll`). All graphical demos use **integer-only math** (no floats) for full ARM compatibility.

## Function Reference — `l_ui.h`

Immediate-mode UI library built on `l_gfx.h`. No heap allocation, no widget tree — declare widgets every frame between `l_ui_begin`/`l_ui_end`. Widget functions return action state (e.g. `l_ui_button` returns 1 if clicked). Generated from doc-comments. Run `.\gen-docs.ps1` to regenerate.

<!-- BEGIN UI REFERENCE -->

| Function | Description |
|----------|-------------|
| **Frame functions** | |
| `l_ui_begin` | Begins a UI frame. Call once per frame before declaring widgets. |
| `l_ui_end` | Ends a UI frame. Handles releasing active widget when mouse released. |
| `l_ui_init` | Initializes a UI context with the default dark theme and font scale 1. |
| **Widgets** | |
| `l_ui_label` | Draws a text label at (x,y). Returns 0 always. |
| `l_ui_button` | Draws a clickable button at (x,y) with given width and height. Returns 1 if clicked this frame. |
| `l_ui_checkbox` | Draws a checkbox at (x,y). *checked is toggled on click. Returns 1 if toggled this frame. |
| `l_ui_slider` | Draws a horizontal slider at (x,y) with given width. *value is clamped to [min_val, max_val]. Returns 1 if value changed. |
| `l_ui_textbox` | Draws a single-line text input at (x,y) with width w. buf is the text buffer, buf_len is max capacity. Returns 1 if text changed. |
| `l_ui_panel` | Draws a panel (filled rectangle with border) at (x,y). Returns 0. |
| `l_ui_separator` | Draws a horizontal separator line at (x,y) with width w. Returns 0. |
| **Auto-Layout Helpers** | |
| `l_ui_column_begin` | Begins a vertical (column) auto-layout at (x,y) with given spacing between widgets. |
| `l_ui_row_begin` | Begins a horizontal (row) auto-layout at (x,y) with given spacing. |
| `l_ui_next` | Advances auto-layout by `size` pixels. Returns the position before advancing (y for column, x for row). |
| `l_ui_layout_end` | Ends the current layout. |

<!-- END UI REFERENCE -->

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
| **hello** | Hello world (README example) | [hello.c](test/hello.c) |
| **file_io** | File read/write (README example) | [file_io.c](test/file_io.c) |
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
| **gfx_demo** | Static drawing — rectangles, circles, text (README example) | [gfx_demo.c](test/gfx_demo.c) |
| **bounce** | Bouncing ball animation — run with `-f` for fullscreen (README example) | [bounce.c](test/bounce.c) |
| **life** | Conway's Game of Life — 80×60 grid, pause/randomize/clear | [life.c](test/life.c) |
| **plasma** | Rainbow plasma — animated sine-wave color cycling | [plasma.c](test/plasma.c) |
| **starfield** | 3D starfield — 200 stars with perspective projection | [starfield.c](test/starfield.c) |
| **fire** | Doom-style fire — bottom-up heat propagation | [fire.c](test/fire.c) |
| **clock** | Analog clock — hour/minute/second hands, ticking in real time | [clock.c](test/clock.c) |

### UI Demos (`l_ui.h`)

| Program | Description | Source |
|---------|-------------|--------|
| **ui_demo** | Full widget showcase — buttons, checkbox, slider, textbox, layout | [ui_demo.c](test/ui_demo.c) |
| **ui_controls** | RGB color mixer — three sliders, hex input, live color preview | [ui_controls.c](test/ui_controls.c) |

### Test Suite

| Program | Assertions | Source |
|---------|-----------|--------|
| **test** | 710 (Linux/ARM/AArch64), 688 (Windows) | [test.c](test/test.c) |
| **gfx_test** | 28 (in-memory pixel buffer tests) | [gfx_test.c](test/gfx_test.c) |
| **ui_test** | UI widget logic tests (simulated canvas) | [ui_test.c](test/ui_test.c) |

## Directory Structure

```
l_os.h          — Core runtime header (strings, I/O, processes, terminal)
l_gfx.h        — Pixel graphics header (drawing, font, canvas)
l_ui.h         — Immediate-mode UI header (widgets, layout, theme)
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
